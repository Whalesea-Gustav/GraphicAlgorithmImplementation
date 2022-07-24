#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <random>
#include <m_common/m_ExamplesVAO.h>
#include <m_common/m_VertexArrayBuffer.h>
#include <cy/cyPoint.h>
#include <cy/cySampleElim.h>

class IndirectPass : public RenderPassV2
{
public:
    using RenderPassV2::RenderPassV2;

    struct alignas(16) IndirectParams {
        glm::mat4 view; //16
                        //16
                        //16
                        //16
        glm::mat4 proj;
                        //16
                        //16
                        //16
                        //16
        int indirect_sample_count;
                        //4
        int indirect_ray_max_steps;
                        //4
        int frame_index;
                        //4
        int trace_max_level;
                        //4
        int width;
                        //4
        int height;
                        //4
        float depth_threshold;
                        //4
        float ray_march_step;
                        //4
        int use_hierarchical_trace;
                        //4
    };

    void setWH(int width, int height)
    {
        this->m_width = width;
        this->m_height = height;
    }

    virtual void InitPass()
    {
        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/SSR/indirect_vs.glsl",
                                             "../../../asset/SSR/indirect_fs.glsl");
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glGenTextures(1, &indirectFluxTexture);
        glBindTexture(GL_TEXTURE_2D, indirectFluxTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, indirectFluxTexture, 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        VAO = VertexArrayBuffer::Create();

        m_pShader->use();
        int indirect_sample_count = 4;
        m_pShader->setInt("IndirectSampleCount", indirect_sample_count);
        m_pShader->setInt("IndirectRayMaxSteps", 64);
        m_pShader->setInt("TraceMaxLevel", 3);
        m_pShader->setInt("Width", m_width);
        m_pShader->setInt("Height", m_height);
        m_pShader->setFloat("DepthThreshold", 1.0f);
        //m_pShader->setFloat("RayMarchingStep", 0.05f);
        m_pShader->setFloat("RayMarchingStep", 1.0f);
        m_pShader->setInt("UseHierarchicalTrace", 1);

        this->setSampleCount(indirect_sample_count);
    }

    void PassBufferDataToUBO()
    {
        glNamedBufferSubData(m_ubo, 0, 176, &m_ubo_buffer_data);
    }

    virtual void RenderPass()
    {
        static int frame_index = 0;
        frame_index++;
        m_pShader->use();
        m_pShader->setInt("FrameIndex", frame_index);

        VAO->Bind();
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glViewport(0, 0, m_width, m_height);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDepthFunc(GL_LESS);

        VAO->UnBind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setSampleCount(int count)
    {
        raw_samples.clear();
        raw_samples.resize(count);
        static std::default_random_engine ::result_type seed = 0;
        const int N = count;
        std::default_random_engine rng{seed + 1};
        std::uniform_real_distribution<float> dis(0, 1);


        std::vector<cy::Point2f> candidateSamples;

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < 10; ++j)
            {
                const float x = dis(rng);
                const float y = dis(rng);
                candidateSamples.emplace_back(x, y);
            }
        }
        std::vector<cy::Point2f> resultSamples(N);
        cy::WeightedSampleElimination<cy::Point2f, float, 2> wse;
        wse.SetTiling(true);
        wse.Eliminate(
            candidateSamples.data(), candidateSamples.size(),
            resultSamples.data(), resultSamples.size());

        for (int i = 0; i < N; ++i)
        {
            raw_samples[i] = glm::vec2(resultSamples[i].Element(0), resultSamples[i].Element(1));
        }

        m_pShader->use();
        for (int i = 0; i < N; ++i)
        {
            m_pShader->setVec2("RawSamples[" + std::to_string(i) + "]", raw_samples[i]);
        }
    }


    virtual ~IndirectPass()
    {
    }
public:

    int m_width;
    int m_height;
    unsigned int m_fbo;
    unsigned int indirectFluxTexture;
    unsigned int depthTexture;
    shared_ptr<VertexArrayBuffer> VAO;

    //abandoned
    unsigned int m_ubo;
    IndirectParams m_ubo_buffer_data;

    std::vector<glm::vec2> raw_samples;
};