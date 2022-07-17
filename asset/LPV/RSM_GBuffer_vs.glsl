#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 wFragPos;
out vec2 TexCoords;
out vec3 wNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightViewProjection;

uniform sampler2D texture_normal1;


void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    wFragPos = vec3(worldPos);
    TexCoords = aTexCoords;
    wNormal =  normalize(mat3(transpose(inverse(model))) * aNormal);

    gl_Position =  lightViewProjection * worldPos;
}