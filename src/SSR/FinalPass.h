#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>
#include <m_common/m_VertexArrayBuffer.h>


class FinalPass : public RenderPassV2
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
        m_pShader = std::make_shared<Shader>("../../../asset/SSR/final_vs.glsl",
                                             "../../../asset/SSR/final_fs.glsl");

        m_pShader->use();
        m_pShader->setBool("EnableDirect", enable_direct);
        m_pShader->setBool("EnableIndirect", enable_indirect);
        m_pShader->setBool("EnableTonemap", enable_tonemap);
        m_pShader->setFloat("Exposure", exposure);

    }


    void RenderPass() override
    {
        m_pShader->use();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glViewport(0, 0, m_width, m_height);
        ExamplesVAO::renderQuad();
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setRenderParameters()
    {
        m_pShader->use();
        m_pShader->setBool("EnableDirect", enable_direct);
        m_pShader->setBool("EnableIndirect", enable_indirect);
        m_pShader->setBool("EnableTonemap", enable_tonemap);
        m_pShader->setFloat("Exposure", exposure);
    }

    virtual ~FinalPass()
    {
    }
public:

    int m_width;
    int m_height;

    bool enable_direct = true;
    bool enable_indirect = true;
    bool enable_tonemap = false;
    float exposure = 1.0;

};