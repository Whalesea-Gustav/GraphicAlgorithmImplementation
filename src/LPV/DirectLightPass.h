#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <random>


class DirectLightPass : public RenderPass
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
        auto* m_pCShader = new ComputeShader("../../../asset/LPV/DirectLight_cs.glsl");
        std::shared_ptr<Shader> new_pShader(m_pCShader);
        m_pShader = new_pShader;

        //Generate Texture for Compute Shader Calculation
        //texture for compute shader calculation
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glGenTextures(1, &(m_texture));
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height,
                     0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D, 0);

        //Global Group Size
        global_group_size.push_back((m_width + local_group_size - 1) / local_group_size);
        global_group_size.push_back((m_height + local_group_size - 1) / local_group_size);
        global_group_size.push_back(1);

    }

    virtual void updateV()
    {
        m_pShader->use();

        m_pShader->setInt("u_OutputImage", 0);
        glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(global_group_size[0], global_group_size[1], global_group_size[2]);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    virtual ~DirectLightPass()
    {

    }

public:
    unsigned int m_texture;

    int m_width;
    int m_height;

    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;
};