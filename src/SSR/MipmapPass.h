#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <random>


class MipmapComputePass : public RenderPassV2
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
        m_pCopyViewZShader = std::make_shared<ComputeShader>("../../../asset/SSR/viewZ_cs.glsl");
        m_pMipmapShader = std::make_shared<ComputeShader>("../../../asset/SSR/mipmap_cs.glsl");

        //Global Group Size
        global_group_size.push_back((m_width + local_group_size - 1) / local_group_size);
        global_group_size.push_back((m_height + local_group_size - 1) / local_group_size);
        global_group_size.push_back(1);

        //Generate Mipmap Texture
        levels = computeLevels(m_width, m_height);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_mipmap);
        glTextureStorage2D(m_mipmap, levels, GL_R32F, m_width, m_height);
    }

    int computeLevels(int w, int h)
    {
        int levels = 1;
        while (w > 1 || h > 1) {
            levels += 1;
            w >>= 1;
            h >>= 1;
        }
        return levels;
    }

    virtual void RenderPass()
    {
        m_pCopyViewZShader->use();
        glBindImageTexture(1, m_mipmap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        glDispatchCompute(global_group_size[0], global_group_size[1], global_group_size[2]);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        m_pMipmapShader->use();
        glm::vec<2, int> pre_res = {m_width, m_height};
        for (int i = 1; i < levels; ++i)
        {
            glm::vec<2, int> cur_res = { std::max(pre_res.x / 2, 1), std::max(pre_res.y / 2, 1)};
            m_pMipmapShader->setVec2("PreSize", glm::vec2(pre_res.x, pre_res.y));
            m_pMipmapShader->setVec2("CurSize", glm::vec2(cur_res.x, cur_res.y));
            glBindImageTexture(0, m_mipmap, i-1, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
            glBindImageTexture(1, m_mipmap, i, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            int x = (cur_res.x + local_group_size - 1) / local_group_size;
            int y = (cur_res.y + local_group_size - 1) / local_group_size;
            glDispatchCompute(x, y, 1);
            pre_res = cur_res;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    virtual ~MipmapComputePass()
    {

    }

public:
    std::shared_ptr<ComputeShader> m_pCopyViewZShader;
    std::shared_ptr<ComputeShader> m_pMipmapShader;

    unsigned int m_mipmap;

    int levels;

    int m_width;
    int m_height;


    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;

};