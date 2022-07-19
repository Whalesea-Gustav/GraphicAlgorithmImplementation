#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>

class SSDOCalculationPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    void setWH(int width, int height)
    {
        this->m_width = width;
        this->m_height = height;
    }


    virtual void InitPass()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/SSDO/SSDO_Calc_vs.glsl",
                                             "../../../asset/SSDO/SSDO_Calc_fs.glsl");

        //Init Framebuffer
        glGenFramebuffers(1, &m_SSDO_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_SSDO_FBO);
        glGenTextures(1, &m_SSDO_Color_Buffer);
        glBindTexture(GL_TEXTURE_2D, m_SSDO_Color_Buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSDO_Color_Buffer, 0);

        unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);
        // create and attach depth buffer (renderbuffer)

        //Order is important
        //1. first glDrawBuffers(3, attachments);
        //2. then bind depth renderbuffer

        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

//        GLuint depth_texture;
//        glGenTextures(1, &depth_texture);
//        glBindTexture(GL_TEXTURE_2D, depth_texture);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
//                     m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
//
//        GLuint attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_DEPTH_ATTACHMENT};
//        glDrawBuffers(4, attachments);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //SSDO sampling Kernel
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
        std::default_random_engine generator;

        for (GLuint i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator)
            );
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            GLfloat scale = GLfloat(i) / 64.0;
            ssdoKernel.push_back(sample);
        }


        m_pShader->use();

        for (int i = 0; i < ssdoKernel.size(); ++i)
        {
            m_pShader->setVec3("samples[" + std::to_string(i) + "]", ssdoKernel[i]);
        }


        // Blur Operation
        glGenFramebuffers(1, &m_Blur_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Blur_FBO);
        glGenTextures(1, &m_Blur_Buffer);
        glBindTexture(GL_TEXTURE_2D, m_Blur_Buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Blur_Buffer, 0);

        m_pBlurShader = std::make_shared<Shader>("../../../asset/SSDO/BlurOperation_vs.glsl",
                                                 "../../../asset/SSDO/BlurOperation_fs.glsl");
    }

    virtual void RenderPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_SSDO_FBO);
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
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, m_SSDO_Color_Buffer);
        m_pBlurShader->setInt("ssdoInput", 8);
        ExamplesVAO::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~SSDOCalculationPass()
    {
    }

public:
    unsigned int m_SSDO_FBO;
    unsigned int m_SSDO_Color_Buffer;
    unsigned int rboDepth;

    std::vector<glm::vec3> ssdoKernel;

    unsigned int m_Blur_FBO;
    unsigned int m_Blur_Buffer;

    int m_width;
    int m_height;

    std::shared_ptr<Shader> m_pBlurShader;
};