#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_Model_v2.h>

class GBufferPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    void setWH(int width, int height)
    {
        this->m_width = width;
        this->m_height = height;
    }

    void setModel(std::shared_ptr<Modelv2<VertexPosNormalTexTanBitan>> pModelv2)
    {
        this->m_pModelv2 = pModelv2;
    }
    virtual void InitPass()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/SSR/GBuffer_vs.glsl",
                                             "../../../asset/SSR/GBuffer_fs.glsl");

        //Init Framebuffer
        glGenFramebuffers(1, &m_gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        glGenTextures(1, &gOutput0);
        glBindTexture(GL_TEXTURE_2D, gOutput0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gOutput0, 0);

        glGenTextures(1, &gOutput1);
        glBindTexture(GL_TEXTURE_2D, gOutput1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gOutput1, 0);


        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        // create and attach depth buffer (renderbuffer)

        //Order is important
        //1. first glDrawBuffers(3, attachments);
        //2. then bind depth renderbuffer

        glGenTextures(1, &gDepthTexture);
        glBindTexture(GL_TEXTURE_2D, gDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexture, 0);

        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void RenderPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, m_width, m_height);
        m_pShader->use();
        m_pModelv2->Draw(*m_pShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~GBufferPass()
    {
        glDeleteFramebuffers(1, &m_gBuffer);
    }
public:
    unsigned int m_gBuffer;
    unsigned int gOutput0, gOutput1;
    unsigned int gDepthTexture;

    std::shared_ptr<Modelv2<VertexPosNormalTexTanBitan>> m_pModelv2;

    int m_width;
    int m_height;
};