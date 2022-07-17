#pragma once
#include <m_common/m_RenderPass.h>
#include <m_common/m_Model.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_UniformBuffer.h>
#include <random>


class GeometryInjectPass : public RenderPass
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

        //Init Shader;
        m_pShader = std::make_shared<Shader>("../../../asset/LPV/GeometryInjectPass_vs.glsl",
                                             "../../../asset/LPV/GeometryInjectPass_fs.glsl");

        m_pShader->use();
        m_pShader->setFloat("u_CellSize", cellSize);
        m_pShader->setVec3("u_MinAABB", min_AABB.x, min_AABB.y, min_AABB.z);
        m_pShader->setInt("u_RSMResolution", m_width);

//        //Generate FrameBuffer
//        glGenFramebuffers(1, &m_gBuffer);
//        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
//
//        //generate 3D texture R, G, B
//        glGenTextures(1, &m_GridR);
//
//        glBindTexture(GL_TEXTURE_3D, m_GridR);
//        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, m_Dimensions.x, m_Dimensions.y, m_Dimensions.z, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, m_GridR, 0, m_Dimensions.z);
//
//        glGenTextures(1, &m_GridG);
//        glBindTexture(GL_TEXTURE_3D, m_GridG);
//        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, m_Dimensions.x, m_Dimensions.y, m_Dimensions.z, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_3D, m_GridG, 0, m_Dimensions.z);

        glGenTextures(1, &m_GridGV);
        glBindTexture(GL_TEXTURE_3D, m_GridGV);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, m_Dimensions.x, m_Dimensions.y, m_Dimensions.z, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    }

    virtual void updateV()
    {
        //clear texture

        std::vector<GLfloat> emptyData(m_Dimensions.x * m_Dimensions.y * m_Dimensions.z * sizeof(float), 0.0);
        glBindTexture(GL_TEXTURE_3D, m_GridGV);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_Dimensions.x, m_Dimensions.y, m_Dimensions.z,
                        GL_RGBA, GL_FLOAT, &emptyData[0]);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glViewport(0, 0, m_Dimensions.x, m_Dimensions.y);

        m_pShader->use();
        glBindImageTexture(0, m_GridGV, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);


        if (m_VPLVAO == -1)
        {
            float *VPLVertices = new float[m_width * m_height];

            unsigned int VAO = 0, VBO = 0;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, m_width * m_height, VPLVertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)(0));

            glBindVertexArray(0);

            m_VPLVAO = VAO;
        }

        glBindVertexArray(m_VPLVAO);
        glDrawArrays(GL_POINTS, 0, m_width * m_height);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~GeometryInjectPass()
    {

    }

public:
    unsigned int m_texture;

    int m_width;
    int m_height;

    glm::ivec3 m_Dimensions;
    glm::vec3 min_AABB;

    unsigned int m_GridGV;
    int m_VPLVAO = -1;

    float cellSize = 1.0f;

    //group info
    int local_group_size = 16;
    std::vector<int> global_group_size;
};