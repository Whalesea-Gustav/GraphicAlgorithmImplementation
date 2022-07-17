#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <m_common/m_Texture.h>
#include <random>


class PropagationPass : public RenderPass
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

        //类似compute shader的效果


        //着色器编译器会进行自动优化

        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/LPV/PropagationPass_vs.glsl",
                                             "../../../asset/LPV/PropagationPass_fs.glsl");
        m_pShader->use();

        m_pShader->setVec3("u_GridDim", m_Dimensions.x, m_Dimensions.y, m_Dimensions.z);
        m_pShader->setBool("u_FirstPropStep", false);

        m_AccumulateGridR.ImageBindUnit = 0;
        m_AccumulateGridG.ImageBindUnit = 1;
        m_AccumulateGridB.ImageBindUnit = 2;
        m_AccumulateGridR.TextureType = m_AccumulateGridG.TextureType = m_AccumulateGridB.TextureType = TextureConfig::m_TextureType::Texture3D;
        m_AccumulateGridR.InternalFormat = m_AccumulateGridG.InternalFormat = m_AccumulateGridB.InternalFormat = GL_RGBA16F;
        m_AccumulateGridR.ExternalFormat = m_AccumulateGridG.ExternalFormat = m_AccumulateGridB.ExternalFormat = GL_RGBA;
        m_AccumulateGridR.DataType = m_AccumulateGridG.DataType = m_AccumulateGridB.DataType = GL_FLOAT;
        m_AccumulateGridR.Width = m_AccumulateGridG.Width = m_AccumulateGridB.Width = m_Dimensions.x;
        m_AccumulateGridR.Height = m_AccumulateGridG.Height = m_AccumulateGridB.Height = m_Dimensions.y;
        m_AccumulateGridR.Depth = m_AccumulateGridG.Depth = m_AccumulateGridB.Depth = m_Dimensions.z;
        m_AccumulateGridR.Type4WrapS = m_AccumulateGridG.Type4WrapS = m_AccumulateGridB.Type4WrapS = GL_CLAMP_TO_EDGE;
        m_AccumulateGridR.Type4WrapT = m_AccumulateGridG.Type4WrapT = m_AccumulateGridB.Type4WrapT = GL_CLAMP_TO_EDGE;

        m_generate_texture(m_AccumulateGridR);
        m_generate_texture(m_AccumulateGridG);
        m_generate_texture(m_AccumulateGridB);

        for (int i = 0; i < m_propagation_num; ++i)
        {
            auto temp_R = TextureConfig();
            auto temp_G = TextureConfig();
            auto temp_B = TextureConfig();

            temp_R.ImageBindUnit = 3;
            temp_G.ImageBindUnit = 4;
            temp_B.ImageBindUnit = 5;
            temp_R.TextureType = temp_G.TextureType = temp_B.TextureType = TextureConfig::m_TextureType::Texture3D;
            temp_R.InternalFormat = temp_G.InternalFormat = temp_B.InternalFormat = GL_RGBA16F;
            temp_R.ExternalFormat = temp_G.ExternalFormat = temp_B.ExternalFormat = GL_RGBA;
            temp_R.DataType = temp_G.DataType = temp_B.DataType = GL_FLOAT;
            temp_R.Width = temp_G.Width = temp_B.Width = m_Dimensions.x;
            temp_R.Height = temp_G.Height = temp_B.Height = m_Dimensions.y;
            temp_R.Depth = temp_G.Depth = temp_B.Depth = m_Dimensions.z;
            temp_R.Type4WrapS = temp_G.Type4WrapS = temp_B.Type4WrapS = GL_CLAMP_TO_EDGE;
            temp_R.Type4WrapT = temp_G.Type4WrapT = temp_B.Type4WrapT = GL_CLAMP_TO_EDGE;

            m_generate_texture(temp_R);
            m_TextureConfigsGridR.push_back(temp_R);
            m_generate_texture(temp_G);
            m_TextureConfigsGridG.push_back(temp_G);
            m_generate_texture(temp_B);
            m_TextureConfigsGridB.push_back(temp_B);
        }


    }

    virtual void updateV()
    {

        //clear texture
        for (int i = 0; i < m_propagation_num; ++i)
        {
            m_clear_texture(m_TextureConfigsGridR[i], GL_TEXTURE_3D);
            m_clear_texture(m_TextureConfigsGridG[i], GL_TEXTURE_3D);
            m_clear_texture(m_TextureConfigsGridB[i], GL_TEXTURE_3D);
        }

        m_clear_texture(m_AccumulateGridR,GL_TEXTURE_3D);
        m_clear_texture(m_AccumulateGridG,GL_TEXTURE_3D);
        m_clear_texture(m_AccumulateGridB,GL_TEXTURE_3D);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, m_Dimensions.x, m_Dimensions.y);

        m_pShader->use();

        m_bind_image_texture(m_AccumulateGridR);
        m_bind_image_texture(m_AccumulateGridG);
        m_bind_image_texture(m_AccumulateGridB);

        for (int i = 0; i < m_propagation_num; ++i)
        {
            m_bind_image_texture(m_TextureConfigsGridR[i]);
            m_bind_image_texture(m_TextureConfigsGridG[i]);
            m_bind_image_texture(m_TextureConfigsGridB[i]);

            DrawVPLVAO();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawVPLVAO()
    {
        if (m_VPLVAO == -1)
        {
            std::vector<glm::vec3> Vertices;

            for (int i = 0; i < m_Dimensions.x; ++i)
            {
                for (int j = 0; j < m_Dimensions.y; ++j)
                {
                    for (int k = 0; k < m_Dimensions.z; ++k)
                    {
                        Vertices.push_back(glm::vec3(i, j, k));
                    }
                }
            }

            unsigned int VAO = 0, VBO = 0;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, Vertices.size() * 3 * sizeof(float), &Vertices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)(0));

            glBindVertexArray(0);

            m_VPLVAO = VAO;
        }

        glBindVertexArray(m_VPLVAO);
        glDrawArrays(GL_POINTS, 0, m_Dimensions.x * m_Dimensions.y * m_Dimensions.z);
        glBindVertexArray(0);
    }

    virtual ~PropagationPass()
    {

    }

public:
    unsigned int m_texture;

    int m_width;
    int m_height;

    glm::ivec3 m_Dimensions;
    glm::vec3 min_AABB;
    glm::vec3 max_AABB;


    //come from two inject passes
    unsigned int m_GridR;
    unsigned int m_GridG;
    unsigned int m_GridB;
    unsigned int m_GridGV;

    int m_VPLVAO = -1;

    float cellSize = 1.0f;

    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;

    TextureConfig m_AccumulateGridR;
    TextureConfig m_AccumulateGridG;
    TextureConfig m_AccumulateGridB;

    int m_propagation_num = 5;

    std::vector<TextureConfig> m_TextureConfigsGridR;
    std::vector<TextureConfig> m_TextureConfigsGridG;
    std::vector<TextureConfig> m_TextureConfigsGridB;

};