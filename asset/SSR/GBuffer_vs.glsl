#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 wPosition;
out vec2 TexCoords;
out vec3 wNormal;
out vec3 wTangent;
out float viewDepth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    //word space
    wPosition = vec3(worldPos);

    wNormal =  normalize(mat3(transpose(inverse(model))) * aNormal);

    TexCoords = aTexCoords;

    wTangent = normalize(vec3(model * vec4(aTangent, 0.0)));

    vec4 viewPos = view * worldPos;
    viewDepth = viewPos.z / viewPos.w;

    gl_Position =  projection * view * worldPos;
}