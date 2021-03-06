#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D hbao;
uniform bool bUseHBAO;
uniform bool bUseLight;
struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};
uniform Light light;

void main()
{
    // Retrieve data from g-buffer
    vec3 FragPos = texture(gPositionDepth, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(hbao, TexCoords).r;

    // Then calculate lighting as usual
    vec3 ambient;
    if (bUseHBAO)
    {
        ambient = vec3(0.3 * AmbientOcclusion); // <-- this is where we use ambient occlusion
    }
    else
    {
        ambient = vec3(0.3);
    }

    vec3 lighting  = ambient;
    vec3 viewDir  = normalize(-FragPos); // Viewpos is (0.0.0)
    // Diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;
    // Attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;

    if (bUseLight)
    {
        lighting += diffuse + specular;
    }
    else
    {
        lighting += Diffuse * 0.7;
    }

    FragColor = vec4(lighting, 1.0);
}