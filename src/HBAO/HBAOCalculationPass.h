#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>
#include <glm/gtc/random.hpp>

class HBAOCalculationPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    void setWH(int width, int height)
    {
        this->m_width = width;
        this->m_height = height;
    }

    void setModel(std::shared_ptr<Model<VertexPosNormalTex>> pModel)
    {
        this->m_pModel = pModel;
    }

    float* generateNoise()
    {
        float *noise = new float[4 * 4 * 4];
        for (int y = 0; y < 4; ++y)
        {
            for (int x = 0; x < 4; ++x)
            {
                glm::vec2 xy = glm::circularRand(1.0f);
                float z = glm::linearRand(0.0f, 1.0f);
                float w = glm::linearRand(0.0f, 1.0f);

                int offset = 4 * (y * 4 + x);
                noise[offset + 0] = xy[0];
                noise[offset + 1] = xy[1];
                noise[offset + 2] = z;
                noise[offset + 3] = w;
            }
        }
        return noise;
    }

    virtual void InitPass()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/HBAO/HBAO_Calc_vs.glsl",
                                             "../../../asset/HBAO/HBAO_Calc_fs.glsl");

        //Init Framebuffer
        glGenFramebuffers(1, &m_HBAO_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_HBAO_FBO);
        glGenTextures(1, &m_HBAO_Color_Buffer);
        glBindTexture(GL_TEXTURE_2D, m_HBAO_Color_Buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_HBAO_Color_Buffer, 0);

        unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);

        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;


        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Generate Noise Texture

        float* HBAONoise = generateNoise();

        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGBA, GL_FLOAT, HBAONoise);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        m_pShader->use();

        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        m_pShader->setInt("u_NoiseTexture", 9);

        m_pShader->setFloat("u_WindowWidth", static_cast<float>(m_width));
        m_pShader->setFloat("u_WindowHeight", static_cast<float>(m_height));
        m_pShader->setFloat("u_Near", zNear);
        m_pShader->setFloat("u_Far", zFar);
        m_pShader->setFloat("u_Fov", glm::radians(fov));
        glm::vec2 FocalLen;
        float fovRad = glm::radians(fov);
        FocalLen[0] = 1.0f / tanf(fovRad * 0.5f) * (static_cast<float>(m_height) / static_cast<float>(m_width));
        FocalLen[1] = 1.0f / tanf(fovRad * 0.5f);
        m_pShader->setVec2("u_FocalLen", FocalLen);


        // Blur Operation
        glGenFramebuffers(1, &m_Blur_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Blur_FBO);
        glGenTextures(1, &m_Blur_Buffer);
        glBindTexture(GL_TEXTURE_2D, m_Blur_Buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Blur_Buffer, 0);

        m_pBlurShader = std::make_shared<Shader>("../../../asset/HBAO/BlurOperation_vs.glsl",
                                                 "../../../asset/HBAO/BlurOperation_fs.glsl");
    }

    virtual void RenderPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_HBAO_FBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, m_width, m_height);
        m_pShader->use();
        ExamplesVAO::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, m_Blur_FBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, m_width, m_height);
        m_pBlurShader->use();
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, m_HBAO_Color_Buffer);
        m_pBlurShader->setInt("hbaoInput", 6);
        ExamplesVAO::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~HBAOCalculationPass()
    {
    }
public:

    //Camera Info
    float zNear = 0.1f, zFar = 100.0f, fov = 60.0f;

    unsigned int m_HBAO_FBO;
    unsigned int m_HBAO_Color_Buffer;
    unsigned int rboDepth;

    //std::vector<glm::vec3> ssaoKernel;
    //std::vector<glm::vec3> ssaoNoise;
    unsigned int noiseTexture;

    unsigned int m_Blur_FBO;
    unsigned int m_Blur_Buffer;

    std::shared_ptr<Model<VertexPosNormalTex>> m_pModel;
    int m_width;
    int m_height;

    std::shared_ptr<Shader> m_pBlurShader;
};