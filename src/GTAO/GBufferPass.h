#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>

class GBufferPass : public RenderPassV2
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
    virtual void InitPass()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/SSAO/GBuffer_vs.glsl",
                                             "../../../asset/SSAO/GBuffer_fs.glsl");

        //Init Framebuffer
        glGenFramebuffers(1, &m_gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        glGenTextures(1, &gAlbedo);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);

        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        glGenTextures(1, &gPositionDepth);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gPositionDepth, 0);

        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
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
        m_pModel->Draw(*m_pShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~GBufferPass()
    {
        glDeleteFramebuffers(1, &m_gBuffer);
    }
public:
    unsigned int m_gBuffer;
    unsigned int gAlbedo, gNormal, gPositionDepth;
    unsigned int gDepthTexture;

    std::shared_ptr<Model<VertexPosNormalTex>> m_pModel;
    int m_width;
    int m_height;
};