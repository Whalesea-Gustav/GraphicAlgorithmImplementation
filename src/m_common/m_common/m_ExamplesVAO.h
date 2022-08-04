#pragma once

#include <m_common/m_VertexArrayBuffer.h>
#include <memory>
#include <m_common/m_Shader.h>

using namespace std;

class ExamplesVAO
{
public:

    static int sphereVAOIndex;
    static int cubeVAOIndex;
    static int quadVAOIndex;
    static int planeVAOIndex;

    static vector<shared_ptr<VertexArrayBuffer>> example_VAO_arr;


    static void renderSphere();
    static void renderCube();
    static void renderQuad();
    static void renderPlane();
    static void renderScene(Shader* shader);
};