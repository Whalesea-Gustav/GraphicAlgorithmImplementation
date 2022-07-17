#version 430 core

in  vec2 v2f_TexCoords;
out vec4 Color_;

uniform sampler2D u_IndirectTexture;
uniform sampler2D u_DirectTexture;
void main()
{
    vec3 TexelColor = texture(u_DirectTexture, v2f_TexCoords).rgb * 0.8f + 0.2 * texture(u_IndirectTexture, v2f_TexCoords).rgb;

    Color_ = vec4(TexelColor, 1.0f);
}