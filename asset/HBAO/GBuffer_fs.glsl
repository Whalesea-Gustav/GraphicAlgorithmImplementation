#version 330 core
layout (location = 0) out vec3 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gPositionDepth;

in vec3 vFragPos;
in vec2 TexCoords;
in vec3 vNormal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;

const float NEAR = 0.1f;
const float FAR = 100.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 回到NDC
    //值得注意的是，这里的分母和投影公式中的分母符号相反
    //是为了获得postive深度信息(因为在Camera的View空间内，顶点坐标Z一般为negative-因为大部分Camera向-z轴构建View矩阵)
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main()
{
    // store info in the gbuffer textures
    gAlbedo = texture(texture_diffuse1, TexCoords).rgb;
    gNormal = vNormal;
    gPositionDepth.xyz = vFragPos;
    gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);
}