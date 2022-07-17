#version 430 core
#pragma optionNV (unroll all)

#define LOCAL_GROUP_SIZE 16

layout (local_size_x = LOCAL_GROUP_SIZE, local_size_y = LOCAL_GROUP_SIZE) in;

layout (rgba32f, binding = 0) uniform writeonly image2D u_OutputImage;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_PositionTexture;
uniform sampler2D u_LightDepthTexture;

uniform mat4  u_LightVPMatrix;
uniform vec3  u_wLightDir;
uniform float u_LightIntensity;

void main()
{
    ivec2 FragPos = ivec2(gl_GlobalInvocationID.xy);
    vec3 wFragNormal = normalize(texelFetch(u_NormalTexture, FragPos, 0).xyz);
    vec3 wFragAlbedo = texelFetch(u_AlbedoTexture, FragPos, 0).xyz;
    vec3 wFragPos = texelFetch(u_PositionTexture, FragPos, 0).xyz;

    vec4 FragPosInLightSpace = u_LightVPMatrix * vec4(wFragPos, 1);

    FragPosInLightSpace /= FragPosInLightSpace.w;
    FragPosInLightSpace.xyz = (FragPosInLightSpace.xyz + 1.0) / 2.0;

    vec2 FragNDCPos4Light = FragPosInLightSpace.xy;

    float Visibility = 0.0f;

    float DirectIllumination = 0.0f;

    float ClosetDepth4Light = texture(u_LightDepthTexture, FragNDCPos4Light).r;

    float Bias = max(0.004f * (1.0f - dot(wFragNormal, u_wLightDir)), 0.004f);
    //Bias = 0.0f;
    Visibility = (FragPosInLightSpace.z - Bias < ClosetDepth4Light) ? 1.0f : 0.0f;

    DirectIllumination = u_LightIntensity * max(dot(u_wLightDir, wFragNormal), 0) * Visibility;

    vec3 Result = wFragAlbedo * DirectIllumination;

    imageStore(u_OutputImage, FragPos, vec4(Result, 1.0));
}