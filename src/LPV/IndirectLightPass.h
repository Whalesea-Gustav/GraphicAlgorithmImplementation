#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <m_common/m_Texture.h>
#include <m_common/m_ExamplesVAO.h>
#include <random>


class IndirectLightPass : public RenderPass
{
public:
    using RenderPass::RenderPass;

    void setWH(int width, int height)
    {
        this->m_width = width;
        this->m_height = height;
    }

    virtual void initV()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/LPV/IndirectLightPass_vs.glsl",
                                             "../../../asset/LPV/IndirectLightPass_fs.glsl");
        m_pShader->use();
        m_pShader->setFloat("u_CellSize", cellSize);
        m_pShader->setVec3("u_MinAABB", min_AABB.x, min_AABB.y, min_AABB.z);

        indirectLight_TextureConfig.Width = m_width;
        indirectLight_TextureConfig.Height = m_height;
        indirectLight_TextureConfig.InternalFormat = GL_RGBA32F;
        indirectLight_TextureConfig.ExternalFormat = GL_RGBA;
        indirectLight_TextureConfig.DataType = GL_FLOAT;
        indirectLight_TextureConfig.isMipmap = false;

        m_generate_texture(indirectLight_TextureConfig);

        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, indirectLight_TextureConfig.TextureID, 0);

        unsigned int rboDepth;

        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void updateV()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_pShader->use();
        ExamplesVAO::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~IndirectLightPass()
    {

    }

public:

    int m_width;
    int m_height;

    unsigned int m_FBO;

    TextureConfig indirectLight_TextureConfig;

    glm::vec3 min_AABB;

    float cellSize = 1.0f;

};