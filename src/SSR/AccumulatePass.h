#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <random>


class AccumulateComputePass : public RenderPassV2
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
        m_pAccumulateShader = std::make_shared<ComputeShader>("../../../asset/SSR/accumulate_cs.glsl");


        //Global Group Size
        global_group_size.push_back((m_width + local_group_size - 1) / local_group_size);
        global_group_size.push_back((m_height + local_group_size - 1) / local_group_size);
        global_group_size.push_back(1);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_src_texture);
        glTextureStorage2D(m_src_texture, 1, GL_RGBA8, m_width, m_height);
        glCreateTextures(GL_TEXTURE_2D, 1, &m_dst_texture);
        glTextureStorage2D(m_dst_texture, 1, GL_RGBA8, m_width, m_height);

        m_ratio = 0.05f;
    }

    virtual void RenderPass()
    {
        m_pAccumulateShader->use();
        m_pAccumulateShader->setFloat("Ratio", m_ratio);

        glBindImageTexture(2, m_dst_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glBindTextureUnit(0, m_src_texture);
        glDispatchCompute(global_group_size[0], global_group_size[1], global_group_size[2]);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        std::swap(m_src_texture, m_dst_texture);
    }

    virtual ~AccumulateComputePass()
    {

    }

public:
    std::shared_ptr<ComputeShader> m_pAccumulateShader;

    unsigned int m_src_texture;
    unsigned int m_dst_texture;

    float m_ratio;
    int m_width;
    int m_height;


    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;

};