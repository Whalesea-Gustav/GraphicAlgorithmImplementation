#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ExamplesVAO.h>

class MainRenderPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    void setWH(int width, int height)
    {
        this->resolution_width = width;
        this->resolution_height = height;
    }


    void InitPass() override
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/CSM/main_rendering_vs.glsl",
                                             "../../../asset/CSM/main_rendering_fs.glsl");

    }


    void RenderPass() override
    {

        m_pShader->use();
        m_pShader->setBool("PCF", bUsePCF);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, resolution_width, resolution_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glCullFace(GL_BACK);
        ExamplesVAO::renderScene(m_pShader.get());

    }

    virtual ~MainRenderPass()
    {

    }
public:
    bool bUsePCF = true;
    int resolution_width;
    int resolution_height;
};