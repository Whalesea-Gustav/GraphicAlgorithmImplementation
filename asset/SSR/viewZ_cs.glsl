#version 460 core
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D Src;

layout(r32f, binding = 1) uniform image2D Dst;

void main() {
    ivec2 res = imageSize(Src);
    ivec2 g_index = ivec2(gl_WorkGroupSize.xy * gl_WorkGroupID.xy + gl_LocalInvocationID.xy);
    if (g_index.x >= res.x || g_index.y >= res.y)
    {
        return;
    }
    vec4 s = imageLoad(Src, g_index);
    imageStore(Dst, g_index, s.z > 0 ? vec4(s.z) : vec4(1.0 / 0.0));
}
