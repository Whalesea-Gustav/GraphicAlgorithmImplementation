#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <random>


class ShadingComputePass : public RenderPass
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
        ComputeShader* m_pCShader = new ComputeShader("../../../asset/RSM/RSM_ShadingCompute_cs.glsl");
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

        init_VPL_weights();
    }


    virtual void updateV()
    {
        m_pShader->use();

        m_pShader->setInt("u_OutputImage", 0);
        glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(global_group_size[0], global_group_size[1], global_group_size[2]);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    virtual ~ShadingComputePass()
    {

    }

    void init_VPL_weights()
    {
        std::default_random_engine e;
        std::uniform_real_distribution<float> u(0, 1);
        for (int i = 0; i < m_VPLNum; ++i)
        {
            float xi_1 = u(e);
            float xi_2 = u(e);
            m_VPLsSampleCoordsAndWeights.emplace_back(xi_1 * sin(2 * AI_MATH_PI * xi_2), xi_1 * cos(2 * AI_MATH_PI * xi_2),
                                                      xi_1 * xi_1, 0);
        }

        m_VPL_ubo = std::make_shared<UniformBuffer>(m_VPLsSampleCoordsAndWeights.data(), m_VPLsSampleCoordsAndWeights.size() * 4 * sizeof(GL_FLOAT));
        m_VPL_ubo->BindBufferBase(0);
    }
public:
    unsigned int m_texture;
    std::shared_ptr<UniformBuffer> m_VPL_ubo;
    std::vector<glm::vec4> m_VPLsSampleCoordsAndWeights;
    int m_VPLNum = 32;
    float m_MaxSampleRadius = 25;
    int m_width;
    int m_height;

    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;

};