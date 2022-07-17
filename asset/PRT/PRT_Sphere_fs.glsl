#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// PRT
uniform vec3 u_PRT_SH_Coef[16];
uniform sampler2D u_PRT_BRDFLut;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    vec3 N = Normal;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // Environment lighting (we now use PRT as the environment term)
    if((abs(N.x) < 0.0001f) && (abs(N.y) < 0.0001f) && (abs(N.z) < 0.0001f))
    {
        FragColor = vec4(0, 0, 0, 1);
        return;
    }

    // Diffuse Calculation
    float Basis[9];
    float x = N.x;
    float y = N.y;
    float z = N.z;
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;

    Basis[0] = 1.f / 2.f * sqrt(1.f / PI);
    Basis[1] = 2.0 / 3.0 * sqrt(3.f / (4.f * PI)) * z;
    Basis[2] = 2.0 / 3.0 * sqrt(3.f / (4.f * PI)) * y;
    Basis[3] = 2.0 / 3.0 * sqrt(3.f / (4.f * PI)) * x;
    Basis[4] = 1.0 / 4.0 * 1.f / 2.f * sqrt(15.f / PI) * x * z;
    Basis[5] = 1.0 / 4.0 * 1.f / 2.f * sqrt(15.f / PI) * z * y;
    Basis[6] = 1.0 / 4.0 * 1.f / 4.f * sqrt(5.f / PI) * (-x2 - z2 + 2 * y2);
    Basis[7] = 1.0 / 4.0 * 1.f / 2.f * sqrt(15.f / PI) * y * x;
    Basis[8] = 1.0 / 4.0 * 1.f / 4.f * sqrt(15.f / PI) * (x2 - z2);

    vec3 PRT_Diffuse = vec3(0,0,0);
    for (int i = 0; i < 9; i++)
    {
        PRT_Diffuse += u_PRT_SH_Coef[i] * Basis[i];
    }


    //BRDF LUT sampling
    vec2 PRT_BRDF_val  = texture(u_PRT_BRDFLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 brdf_Integrate = (F * PRT_BRDF_val.x + PRT_BRDF_val.y);

    // Env Specular Calculation
    x = R.x;
    y = R.y;
    z = R.z;
    x2 = x * x;
    y2 = y * y;
    z2 = z * z;
    Basis[0] = 1.f / 2.f * sqrt(1.f / PI);
    Basis[1] = sqrt(3.f / (4.f * PI)) * z;
    Basis[2] = sqrt(3.f / (4.f * PI)) * y;
    Basis[3] = sqrt(3.f / (4.f * PI)) * x;
    Basis[4] = 1.f / 2.f * sqrt(15.f / PI) * x * z;
    Basis[5] = 1.f / 2.f * sqrt(15.f / PI) * z * y;
    Basis[6] = 1.f / 4.f * sqrt(5.f / PI) * (-x2 - z2 + 2 * y2);
    Basis[7] = 1.f / 2.f * sqrt(15.f / PI) * y * x;
    Basis[8] = 1.f / 4.f * sqrt(15.f / PI) * (x2 - z2);

    vec3 PRT_Specular = vec3(0,0,0);
    for (int i = 0; i < 9; i++)
    {
        PRT_Specular += u_PRT_SH_Coef[i] * Basis[i];
    }

    PRT_Specular = PRT_Specular * brdf_Integrate;
    vec3 ambient = (kD * PRT_Diffuse + PRT_Specular) * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color , 1.0);
}