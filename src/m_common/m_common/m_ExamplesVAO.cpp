#include <m_common/m_ExamplesVAO.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

int ExamplesVAO::sphereVAOIndex = -1;
int ExamplesVAO::cubeVAOIndex = -1;
int ExamplesVAO::quadVAOIndex = -1;
int ExamplesVAO::planeVAOIndex = -1;

vector<shared_ptr<VertexArrayBuffer>> ExamplesVAO::example_VAO_arr = {};

void ExamplesVAO::renderSphere()
{
    if (ExamplesVAO::sphereVAOIndex == -1)
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            } else {
                for (int x = X_SEGMENTS; x >= 0; --x) {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        std::shared_ptr<VertexBuffer> VBO = VertexBuffer::Create(data.data(), data.size() * sizeof(float));
        BufferLayout layout{{"position", OpenGLDataType::Float, 3},
                            {"normal", OpenGLDataType::Float, 3},
                            { "texcoord", OpenGLDataType::Float, 2}};
        VBO->SetLayout(layout);
        std::shared_ptr<IndexBuffer> IBO = IndexBuffer::Create(indices.data(), indices.size());
        std::shared_ptr<VertexArrayBuffer> VAO = VertexArrayBuffer::Create(VBO, IBO);

        ExamplesVAO::sphereVAOIndex = ExamplesVAO::example_VAO_arr.size();
        ExamplesVAO::example_VAO_arr.push_back(VAO);
    }

    auto sphereVAO = ExamplesVAO::example_VAO_arr[ExamplesVAO::sphereVAOIndex];

    sphereVAO->Bind();
    unsigned int indexCount = sphereVAO->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    sphereVAO->UnBind();
}

void ExamplesVAO::renderCube()
{
    if (ExamplesVAO::cubeVAOIndex == -1)
    {
        float data[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        unsigned int data_count  = 288;
        unsigned int indices[] =
                {
                0, 1, 2,
                3, 4, 5,
                6, 7, 8,
                9, 10, 11,
                12, 13, 14,
                15, 16, 17,
                18, 19, 20,
                21, 22, 23,
                24, 25, 26,
                27, 28, 29,
                30, 31, 32,
                33, 34, 35
                };
        unsigned int indices_count = 36;

        std::shared_ptr<VertexBuffer> VBO = VertexBuffer::Create(data, data_count * sizeof(float));
        BufferLayout layout{{"position", OpenGLDataType::Float, 3},
                            {"normal", OpenGLDataType::Float, 3},
                            { "texcoord", OpenGLDataType::Float, 2}};
        VBO->SetLayout(layout);
        std::shared_ptr<IndexBuffer> IBO = IndexBuffer::Create(indices, indices_count);
        std::shared_ptr<VertexArrayBuffer> VAO = VertexArrayBuffer::Create(VBO, IBO);

        ExamplesVAO::cubeVAOIndex = ExamplesVAO::example_VAO_arr.size();
        ExamplesVAO::example_VAO_arr.push_back(VAO);
    }
    auto cubeVAOptr = ExamplesVAO::example_VAO_arr[ExamplesVAO::cubeVAOIndex];
    cubeVAOptr->Bind();
    unsigned int indexCount = cubeVAOptr->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    cubeVAOptr->UnBind();
}

void ExamplesVAO::renderQuad()
{
    if (ExamplesVAO::quadVAOIndex == -1)
    {
        float data[] =
                {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                };
        unsigned int data_count  = 20;
        unsigned int indices[] =
                {
                    0, 1, 2, 3
                };
        unsigned int indices_count = 4;

        std::shared_ptr<VertexBuffer> VBO = VertexBuffer::Create(data, data_count * sizeof(float));
        BufferLayout layout{{"position", OpenGLDataType::Float, 3},
                            { "texcoord", OpenGLDataType::Float, 2}};
        VBO->SetLayout(layout);
        std::shared_ptr<IndexBuffer> IBO = IndexBuffer::Create(indices, indices_count);
        std::shared_ptr<VertexArrayBuffer> VAO = VertexArrayBuffer::Create(VBO, IBO);

        ExamplesVAO::quadVAOIndex = ExamplesVAO::example_VAO_arr.size();
        ExamplesVAO::example_VAO_arr.push_back(VAO);
    }
    auto quadVAOptr = ExamplesVAO::example_VAO_arr[ExamplesVAO::quadVAOIndex];
    quadVAOptr->Bind();
    unsigned int indexCount = quadVAOptr->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    quadVAOptr->UnBind();
}

void ExamplesVAO::renderPlane() {
    if (ExamplesVAO::planeVAOIndex == -1)
    {
        float planeVertices[] = {
                // positions            // normals         // texcoords
                25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
                -25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
                -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
                25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
                -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
                25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
        };

        unsigned int indices[] =
                {
                        0, 1, 2, 3, 4, 5
                };
        unsigned int indices_count = 6;

        std::shared_ptr<VertexBuffer> VBO = VertexBuffer::Create(planeVertices, sizeof(planeVertices));
        BufferLayout layout{{"position", OpenGLDataType::Float, 3},
                            {"normal", OpenGLDataType::Float, 3},
                            { "texcoord", OpenGLDataType::Float, 2}};
        VBO->SetLayout(layout);
        std::shared_ptr<IndexBuffer> IBO = IndexBuffer::Create(indices, indices_count);
        std::shared_ptr<VertexArrayBuffer> VAO = VertexArrayBuffer::Create(VBO, IBO);

        ExamplesVAO::planeVAOIndex = ExamplesVAO::example_VAO_arr.size();
        ExamplesVAO::example_VAO_arr.push_back(VAO);
    }

    auto planeVAOptr = ExamplesVAO::example_VAO_arr[ExamplesVAO::planeVAOIndex];
    planeVAOptr->Bind();
    unsigned int indexCount = planeVAOptr->GetIndexBuffer()->GetCount();
    glDrawArrays(GL_TRIANGLES, 0, indexCount);
    planeVAOptr->UnBind();
}

static std::random_device device;
static std::mt19937 generator = std::mt19937(device());

void ExamplesVAO::renderScene(Shader *shader) {
    glm::mat4 model = glm::mat4(1.0f);

    shader->use();
    shader->setMat4("model", model);
    renderPlane();

    static std::vector<glm::mat4> modelMatrices;
    if (modelMatrices.empty())
    {
        for (int i = 0; i < 10; ++i)
        {
            static std::uniform_real_distribution<float> offsetDistribution = std::uniform_real_distribution<float>(-10, 10);
            static std::uniform_real_distribution<float> scaleDistribution = std::uniform_real_distribution<float>(1.0, 2.0);
            static std::uniform_real_distribution<float> rotationDistribution = std::uniform_real_distribution<float>(0, 180);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(offsetDistribution(generator), offsetDistribution(generator) + 10.0f, offsetDistribution(generator)));
            model = glm::rotate(model, glm::radians(rotationDistribution(generator)), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
            model = glm::scale(model, glm::vec3(scaleDistribution(generator)));
            modelMatrices.push_back(model);
        }
    }

    for (const auto& local_model : modelMatrices)
    {
        shader->setMat4("model", local_model);
        renderCube();
    }
}


