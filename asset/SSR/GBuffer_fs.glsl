#version 460 core
layout (location = 0) out vec4 gOutput0;
layout (location = 1) out vec4 gOutput1;

in vec3 wPosition;
in vec2 TexCoords;
in vec3 wNormal;
in vec3 wTangent;
in float viewDepth;

uniform sampler2D AlbedoMap; //Albedo
uniform sampler2D NormalMap;

vec2 octWrap(vec2 v)
{
    return (1.0 - abs(v.yx)) * (all(greaterThanEqual(v.xy, vec2(0.0))) ? vec2(1.0) : vec2(-1.0));
}

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
// https://www.shadertoy.com/view/llfcRl
vec2 encodeNormal(vec3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : octWrap(n.xy);
    return n.xy;
}

void main()
{
    vec3 world_normal = normalize(wNormal);
    vec3 world_tangent = normalize(wTangent);
    vec3 world_bitangent = normalize(cross(world_normal, world_tangent));

    vec3 albedo = pow(texture(AlbedoMap, TexCoords).rgb, vec3(2.2));
    vec3 local_normal = normalize(2 * texture(NormalMap, TexCoords).xyz - vec3(1));
    vec3 normal = local_normal.x * world_tangent + local_normal.y * world_bitangent + local_normal.z * world_normal;

    vec2 oct_normal = encodeNormal(normal);

    // 1/PI for cos weight hemisphere sampling
    vec3 color  = albedo / 3.14159265;
    float color1 = uintBitsToFloat(packHalf2x16(color.rb));
    float color2 = color.g;

    //ok, just need two rgba32f texture
    gOutput0 = vec4(wPosition, normal.x);
    gOutput1 = vec4(color1, color2, viewDepth, normal.y);
}