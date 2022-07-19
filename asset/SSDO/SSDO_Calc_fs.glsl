#version 330 core
out vec3 FragColor;
in vec2 TexCoords;

uniform sampler2D gAlbedo;
uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 1.0;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1200.0f/4.0f, 900.0f/4.0f);

uniform mat4 projection;

void main()
{
    // Get input for SSAO algorithm
    vec3 fragPos = texture(gPositionDepth, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Iterate over the sample kernel and calculate indirectLight
    vec3 indirectLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 m_sample = TBN * samples[i];// From tangent to tangent-space
        m_sample = fragPos + m_sample * radius;

        vec4 offset = vec4(m_sample, 1.0);
        offset = projection * offset;// from view to clip-space
        offset.xyz /= offset.w;// perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        if (offset.x > 1.0 || offset.x < 0 || offset.y > 1.0 || offset.x < 0)
        {
            continue;
        }

        // get sample depth
        // 确切来说，是projected Sample Info
        float sampleDepth = -texture(gPositionDepth, offset.xy).w;// Get depth value of kernel sample
        vec3 sampleNormal = texture(gNormal, offset.xy).xyz;
        vec3 samplePos = texture(gPositionDepth, offset.xy).xyz;
        vec3 sampleColor = texture(gAlbedo, offset.xy).xyz;

        //sampleDepth >= m_sample.z (注意值都是负的)，此时采样点m_sample被遮挡
        indirectLight += (sampleDepth >= m_sample.z ? 1.0 : 0.0) * max(dot(sampleNormal, normalize(fragPos - samplePos)), 0.0) * sampleColor;
    }

    //adjust for rendering
    indirectLight /= 1.0;

    FragColor = indirectLight;
}