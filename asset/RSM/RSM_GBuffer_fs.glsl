#version 330 core
layout (location = 0) out vec3 gFlux;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition;

in vec3 FragPosInViewSpace;
in vec2 TexCoords;
in vec3 Normal;

uniform vec3 lightColor = vec3(1.0);
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;

void main()
{
    // store info in the gbuffer textures
    gFlux = lightColor * texture(texture_diffuse1, TexCoords).rgb;
    gNormal = Normal;
    gPosition = FragPosInViewSpace;
}