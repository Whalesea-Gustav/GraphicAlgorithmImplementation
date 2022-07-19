#version 330 core
in vec2 TexCoords;

out vec3 fragColor;

uniform sampler2D ssdoInput;
const int blurSize = 4; // use size of noise texture (4x4)

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssdoInput, 0));
    vec3 result = vec3(0.0);
    for (int x = 0; x < blurSize; ++x)
    {
        for (int y = 0; y < blurSize; ++y)
        {
            vec2 offset = (vec2(-2.0) + vec2(float(x), float(y))) * texelSize;
            result += texture(ssdoInput, TexCoords + offset).rgb;
        }
    }

    fragColor = result / float(blurSize * blurSize);
}