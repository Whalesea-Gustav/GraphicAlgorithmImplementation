#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <m_common/m_VertexBuffer.h>

struct VertexPosNormalTexTanBitanBone {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[4];
    //weights from each bone
    float m_Weights[4];

    BufferLayout layoutInfo;
};

struct VertexPosNormalTex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoord;

    //static members don`t contribute to sizeof operator
    static BufferLayout layoutInfo;

    VertexPosNormalTex() {};
};

BufferLayout VertexPosNormalTex::layoutInfo({
            {"Position", OpenGLDataType::Float, 3, false},
            {"Normal",   OpenGLDataType::Float, 3, false},
            {"TexCoord", OpenGLDataType::Float, 2, false}});

struct VertexPosNormalTexTanBitan{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoord;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;

    static BufferLayout layoutInfo;

    VertexPosNormalTexTanBitan() {};
};

BufferLayout VertexPosNormalTexTanBitan::layoutInfo({
    {"Position", OpenGLDataType::Float, 3, false},
    {"Normal",   OpenGLDataType::Float, 3, false},
    {"TexCoord", OpenGLDataType::Float, 2, false},
    {"Tangent",   OpenGLDataType::Float, 3, false},
    {"Bitangent",   OpenGLDataType::Float, 3, false} });