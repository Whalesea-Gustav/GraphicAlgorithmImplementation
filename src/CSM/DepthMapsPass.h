#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ExamplesVAO.h>

class DepthMapsPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    void setWH(int width, int height)
    {
        this->resolution_width = width;
        this->resolution_height = height;
    }

    void setCameraZ(float _zFar, float _zNear)
    {
        zFar = _zFar;
        zNear = _zNear;
    }

    std::vector<glm::vec4> getFrustumCornersWorldSpace(glm::mat4 proj, glm::mat4 view) {

        auto inv = glm::inverse(proj * view);

        std::vector<glm::vec4> frustumCorners;

        for (uint32_t x = 0; x < 2; ++x)
        {
            for (uint32_t y = 0; y < 2; ++y)
            {
                for (uint32_t z = 0; z < 2; ++z)
                {
                    glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }

        return frustumCorners;
    }

    glm::mat4 getLightSpaceMatrix(float nearPlane, float farPlane)
    {
        glm::mat4 proj = glm::perspective(glm::radians(cameraZoom), aspectRatio, nearPlane, farPlane);

        auto corners = getFrustumCornersWorldSpace(proj, cameraView);

        glm::vec3 center = glm::vec3(0.0f);

        for (auto& v : corners)
        {
            center += glm::vec3(v);
        }

        center /= corners.size();

        auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

        //find aabb of frustum in lightView space

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::min();
        for (const auto& v : corners)
        {
            const auto trf = lightView * v;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        // Tune this parameter according to the scene
        constexpr float zMult = 10.0f;
        if (minZ < 0)
        {
            minZ *= zMult;
        }
        else
        {
            minZ /= zMult;
        }
        if (maxZ < 0)
        {
            maxZ /= zMult;
        }
        else
        {
            maxZ *= zMult;
        }

        const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

        return lightProjection * lightView;
    }

    std::vector<glm::mat4> getLightSpaceMatrices()
    {
        std::vector<glm::mat4> ret;
        for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
        {
            if (i == 0)
            {
                ret.push_back(getLightSpaceMatrix(zNear, shadowCascadeLevels[i]));
            }
            else if (i < shadowCascadeLevels.size())
            {
                ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
            }
            else
            {
                ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], zFar));
            }
        }
        return ret;
    }

    void InitPass() override
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/CSM/shadowmap_vs.glsl",
                                             "../../../asset/CSM/shadowmap_fs.glsl",
                                             "../../../asset/CSM/shadowmap_gs.glsl");

        shadowCascadeLevels = { zFar / 50.0f, zFar / 25.0f, zFar / 10.0f, zFar / 2.0f };

        glGenFramebuffers(1, &lightFBO);

        glGenTextures(1, &lightDepthMaps);
        glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);
        glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, resolution_width, resolution_height, int(shadowCascadeLevels.size()) + 1,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

        glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMaps, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
            throw 0;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenBuffers(1, &matricesUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
        // bindingpoint in shader layout (std140, binding = 0)
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);



    }


    void RenderPass() override
    {
        // UBO setup
        auto lightMatrices = getLightSpaceMatrices();
        glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
        for (size_t i = 0; i < lightMatrices.size(); ++i)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        m_pShader->use();
        glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, resolution_width, resolution_height);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        ExamplesVAO::renderScene(m_pShader.get());
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~DepthMapsPass()
    {

    }
public:
    uint32_t lightFBO;
    uint32_t lightDepthMaps;
    uint32_t matricesUBO;

    std::vector<float> shadowCascadeLevels;

    glm::mat4 cameraView;
    float cameraZoom;
    float aspectRatio;
    float zFar;
    float zNear;

    glm::vec3 lightDir;



    int resolution_width;
    int resolution_height;
};