#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_Model_v2.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>
#include <glm/gtc/random.hpp>

class ShadowMapPass : public RenderPassV2
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
        m_pShader = std::make_shared<Shader>("../../../asset/SSR/LightPass_vs.glsl",
                                             "../../../asset/SSR/LightPass_fs.glsl");

        //Init Framebuffer
        glGenFramebuffers(1, &m_shadowmap_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowmap_FBO);

        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        //Disable Color Rendering
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    virtual void RenderPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowmap_FBO);

        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, m_width, m_height);
        m_pShader->use();

        m_pModelv2->Draw(*m_pShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    virtual ~ShadowMapPass()
    {
    }

public:

    unsigned int m_shadowmap_FBO;
    unsigned int depthTexture;

    std::shared_ptr<Modelv2<VertexPosNormalTexTanBitan>> m_pModelv2;

    int m_width;
    int m_height;

};