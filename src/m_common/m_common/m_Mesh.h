#include <GL/glew.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <m_common/m_vertex.h>
#include <m_common/m_Shader.h>

#include <m_common/m_VertexArrayBuffer.h>
#include <m_common/m_UniformBuffer.h>

#include <string>
#include <vector>

using namespace std;

struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct Material
{
    glm::vec4 Ka;
    glm::vec4 Kd;
    glm::vec4 Ks;
};

template<class Vertex>
class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    vector<shared_ptr<UniformBuffer>> UBOs;
    shared_ptr<VertexArrayBuffer> VAO;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }


    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, Material mat)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        auto mat_ptr = UniformBuffer::Create((void *) & mat, sizeof(mat));
        this->UBOs.push_back(mat_ptr);

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader &shader)
    {
        shader.use();
        // bind appropriate textures
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to string
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string
            else if(name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to string

            // now set the sampler to the correct texture unit
            shader.setInt(name+number, i); //glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }



        // draw mesh
        this->VAO->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int> (indices.size()), GL_UNSIGNED_INT, 0);
        this->VAO->UnBind();

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data
    // unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        shared_ptr<VertexBuffer> VBO = VertexBuffer::Create(this->vertices.data(), this->vertices.size() * sizeof(Vertex));
        VBO->SetLayout(Vertex::layoutInfo);
        shared_ptr<IndexBuffer> IBO = IndexBuffer::Create(this->indices.data(), this->indices.size());

        this->VAO = VertexArrayBuffer::Create(VBO, IBO);
    }
};