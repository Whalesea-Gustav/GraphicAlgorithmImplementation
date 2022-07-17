#version 330 core
layout (location = 0) out vec3 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition;

in vec3 wFragPos;
in vec2 TexCoords;
in vec3 wNormal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;

void main()
{
    // store info in the gbuffer textures
    gAlbedo = texture(texture_diffuse1, TexCoords).rgb;
    gNormal = wNormal;
    gPosition = wFragPos;
}