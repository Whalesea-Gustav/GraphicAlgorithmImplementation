#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_normal1;

void main()
{
    vec3 d = texture(texture_diffuse1, TexCoords).rgb;
    vec3 a = texture(texture_diffuse2, TexCoords).rgb;
    vec3 bump = texture(texture_normal1, TexCoords).rgb;

    FragColor =vec4((d + a + bump) / 2, 1.0);
}