#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>

class SSAOApplyPass : public RenderPass
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
        m_pShader = std::make_shared<Shader>("../../../asset/SSAO/ApplySSAO_vs.glsl",
                                             "../../../asset/SSAO/ApplySSAO_fs.glsl");

    }


    virtual void updateV()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, m_width, m_height);
        m_pShader->use();
        ExamplesVAO::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~SSAOApplyPass()
    {
    }
public:

    int m_width;
    int m_height;

};