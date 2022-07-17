#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform Mat
{
    vec4 Ka;
    vec4 Kd;
    vec4 Ks;
};


struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

//texture
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

uniform Light light;
uniform vec3 viewPos;
uniform bool blinn;
uniform float shininess;
uniform int sampleRadius;

// Shadow map related variables
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

#define FILTER_SIZE_INT 20

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

float rand_1to1(highp float x ) {
    // -1 -1
    return fract(sin(x)*10000.0);
}

float rand_2to1(vec2 uv ) {
    // 0 - 1
    const float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
    return fract(sin(sn) * c);
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples(vec2 randomSeed) {

    float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

    float angle = rand_2to1( randomSeed ) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

void uniformDiskSamples(vec2 randomSeed) {

    float randNum = rand_2to1(randomSeed);
    float sampleX = rand_1to1( randNum ) ;
    float sampleY = rand_1to1( sampleX ) ;

    float angle = sampleX * PI2;
    float radius = sqrt(sampleY);

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        poissonDisk[i] = vec2( radius * cos(angle) , radius * sin(angle)  );

        sampleX = rand_1to1( sampleY ) ;
        sampleY = rand_1to1( sampleX ) ;

        angle = sampleX * PI2;
        radius = sqrt(sampleY);
    }
}

float Bias(){
    //ref : https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/#_7
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
    return  bias;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //conver xyz to [0,1]^3
    //depthmap have value [0,1], it conver [-1,1] to [0, 1] correspondingly
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -sampleRadius; x <= sampleRadius; ++x)
    {
        for (int y = -sampleRadius; y <= sampleRadius; ++y)
        {
            float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += currentDepth - bias > sampleDepth ? 1.0 : 0.0;
        }
    }

    shadow /= (2 * sampleRadius + 1) * (2 * sampleRadius + 1);

    return shadow;
}

float findBlocker(vec2 uv, float zReceiver)
{
    poissonDiskSamples(uv);
    float avgDepth = 0.0;
    int count = 0;
    float bias = Bias();
    vec2 filterSize = 1.0 / textureSize(shadowMap, 0) * FILTER_SIZE_INT;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float closestDepth = texture(shadowMap, uv + poissonDisk[i] * filterSize).r;

        if (zReceiver-bias > closestDepth)
        {
            avgDepth += closestDepth;
            count++;
        }
    }
    if (count == 0)
    {
        return -1.0;
    }
    else
    {
        return avgDepth / float(count);
    }
}
float PCF_Kernel(vec2 uv, float zReceiver, float penumbraScale)
{
    poissonDiskSamples(uv);

    float bias = Bias();
    float visibility = 0.0;

    vec2 filterSize = 1.0 / textureSize(shadowMap, 0) * FILTER_SIZE_INT;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float closestDepth = texture(shadowMap, uv + poissonDisk[i] * filterSize * penumbraScale).r;

        if (zReceiver - bias <= closestDepth)
        {
            visibility += 1.0;
        }
    }

    return visibility / float(NUM_SAMPLES);
}

float PCSS(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //conver xyz to [0,1]^3
    //depthmap have value [0,1], it conver [-1,1] to [0, 1] correspondingly
    projCoords = projCoords * 0.5 + 0.5;

    float zBlocker = findBlocker(projCoords.xy, projCoords.z);

    if (zBlocker < EPS)
    {
        return 1.0;
    }
    else if (zBlocker  > 1.0 + EPS)
    {
        return 0.0;
    }

    float penumbraScale = (projCoords.z - zBlocker) / zBlocker;

    return PCF_Kernel(projCoords.xy, projCoords.z, penumbraScale);
}

void main()
{
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = light.ambient * color;
    // diffuse
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }
    vec3 specular =light.specular * spec; // assuming bright white light color

    // attenuation
    float distance    = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    // shadow
    //float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);

    float visibility = PCSS(fs_in.FragPosLightSpace);

    vec3 lighting = ambient + visibility * (diffuse + specular);

    FragColor = vec4(lighting, 1.0);
}