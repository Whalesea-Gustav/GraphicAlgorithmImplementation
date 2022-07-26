#version 460 core
#define PI 3.14159265

layout(location = 0) in vec2 iUV;

layout(location = 0) out vec4 oFragColor;

#define M_EPS 1e-6
#define M_PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309


uniform mat4 View;
uniform mat4 Proj;
uniform mat4 VP;
uniform int IndirectSampleCount = 4;
uniform int IndirectRayMaxSteps;
uniform int FrameIndex;
uniform int TraceMaxLevel;
uniform int Width;
uniform int Height;
uniform float DepthThreshold;
uniform float RayMarchingStep;
uniform int UseHierarchicalTrace;

uniform vec3 cameraPos;
uniform vec3 lightDir;
uniform vec3 lightRadiance;
uniform mat4 lightSpaceMatrix;

uniform vec2 RawSamples[32];//sample count with IndirectSampleCount

uniform sampler2D Direct;
uniform sampler2D DepthMap;
uniform sampler2D GBuffer0;
uniform sampler2D GBuffer1;
uniform sampler2D ViewDepth;

vec3 decodeNormal(vec2 f)
{
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0, 1);
    n.xy += all(greaterThanEqual(n.xy, vec2(0.0))) ? vec2(-t) : vec2(t);
    return normalize(n);
}

// http://advances.realtimerendering.com/s2015/
// stochastic screen-space reflections
bool hierarchicalRayTrace(in float jitter, in vec3 ori, in vec3 dir, out vec2 res_uv){
    //basic idea: trace ray in view space
    //stackless ray walk of min-z pyramid
    //mip = 0
    //while(level > -1 )
    //    step through current cell
    //    if(above z plane) ++level;
    //    if(below z plane) --level;

    float ray_len = (ori.z + dir.z < 0.1) ? (0.1 - ori.z) / dir.z : 1;
    vec3 end = ori + dir * ray_len;

    vec4 ori_clip = Proj * vec4(ori, 1.0);
    vec4 end_clip = Proj * vec4(end, 1.0);

    float inv_ori_w = 1.0 / ori_clip.w;
    float inv_end_w = 1.0 / end_clip.w;

    vec2 ori_ndc = ori_clip.xy * inv_ori_w;//-1 ~ 1
    vec2 end_ndc = end_clip.xy * inv_end_w;
    vec2 extend_ndc = end_ndc - ori_ndc;

    int width = Width;
    int height = Height;
    vec2 delta_pixel = 0.5 * extend_ndc * vec2(width, height);//not multiply 0.5 is also ok

    //step along x or y which has bigger differential
    bool swap_xy = false;
    if (abs(extend_ndc.x * width) < abs(extend_ndc.y * height)){
        swap_xy = true;
        ori_ndc = ori_ndc.yx;
        end_ndc = end_ndc.yx;
        extend_ndc = extend_ndc.yx;
        delta_pixel = delta_pixel.yx;
        width = Height;
        height = Width;
    }
    //ndc x delta per pixel
    float dx = sign(extend_ndc.x) * 2 / width;
    //normalize dx
    dx *= abs(delta_pixel.x) / length(delta_pixel);
    float dy = extend_ndc.y / extend_ndc.x * dx;
    // unit delta ndc per pixel
    vec2 dp = vec2(dx, dy);

    float ori_z_over_w = ori.z * inv_ori_w;//-1 ~ 1
    float end_z_over_w = end.z * inv_end_w;
    // z / w is linear in ndc coord
    float dz_over_w = (end_z_over_w - ori_z_over_w) / extend_ndc.x * dx;
    // 1 / w is linear in ndc coord
    float dinv_w = (inv_end_w - inv_ori_w) / extend_ndc.x * dx;

    #define HIT_STEPS 3
    #ifndef NO_MIPMAP
    //start from level 0
    int level = 0;
    //ray advance step, will change according to current level
    float level_advance_dist[16];
    //measure in pixel size
    float step = RayMarchingStep;
    int total_steps = IndirectRayMaxSteps;
    int steps_count = 0;
    level_advance_dist[0] = jitter * step;
    //if hit a level than keep search this level for some times
    int hit = 0;
    while (true){
        if (++steps_count > total_steps)
        return false;

        //get curret ray advance start pos
        float t = level_advance_dist[level] + step;
        //update ray advance dist after current step
        level_advance_dist[level] = t;

        //compute current ray ndc
        vec2 p = ori_ndc + t * dp;
        float z_over_w = ori_z_over_w + t * dz_over_w;
        float inv_w = inv_ori_w + t * dinv_w;

        vec2 ndc = swap_xy ? p.yx : p;
        vec2 uv = ndc * 0.5 + 0.5;
        if (any(notEqual(clamp(uv, vec2(0), vec2(1)), uv)))
        return false;

        float ray_depth = z_over_w / inv_w - 0.1;
        float cell_z = textureLod(ViewDepth, uv, level).r;

        if (ray_depth < cell_z){
            if (--hit < 0){
                level = min(level + 1, TraceMaxLevel);
                level_advance_dist[level] = t;
                step = float(1 << level) * RayMarchingStep;
            }
            continue;
        }
        //only check if level is zero and check depth threshould handle if ray is in shadow
        if (level == 0){
            bool find = (ray_depth - DepthThreshold) <= cell_z;
            if (find){
                res_uv = uv;
                return true;
            }
        }
        hit = HIT_STEPS;
        level = max(0, level - 1);
        //update new level's advance distance
        level_advance_dist[level] = t - step;
        step = float(1 << level) * RayMarchingStep;
    }
    return false;
    #else
    //this not use mipmap but trace in ndc space
    float step = RayMarchingStep;
    int total_steps = IndirectRayMaxSteps;
    int steps_count = 0;
    float t = jitter * step;
    while (++steps_count < total_steps){
        vec2 p = ori_ndc + t * dp;
        float z_over_w = ori_z_over_w + t * dz_over_w;
        float inv_w = inv_ori_w + t * dinv_w;

        vec2 ndc = swap_xy ? p.yx : p;
        vec2 uv = ndc * 0.5 + 0.5;
        if (any(notEqual(clamp(uv, vec2(0), vec2(1)), uv)))
        return false;

        float ray_depth = z_over_w / inv_w - 0.1;
        float cell_z = textureLod(ViewDepth, uv, 0).r;

        if (ray_depth > cell_z){
            res_uv = uv;
            return ray_depth - DepthThreshold <= cell_z;
        }

        t += step;
    }
    return false;
    #endif
}

//linear ray marching in camera view space
bool linearRayMarching(in float jitter, in vec3 ori, in vec3 dir, out vec2 res_uv){
    float step = RayMarchingStep;
    float t = jitter * step;
    for (int i = 0; i < IndirectRayMaxSteps; i++){
        vec3 p = ori + dir * (t + 0.5 * step);
        t += step;
        vec4 clip_pos = Proj * vec4(p, 1.0);
        vec2 uv = clip_pos.xy/clip_pos.w * 0.5 + 0.5;
        float ray_depth = p.z;
        if (any(notEqual(clamp(uv, vec2(0), vec2(1)), uv)))
        return false;
        float z = textureLod(ViewDepth, uv, 0).r;
        if (ray_depth >= z){
            res_uv = uv;
            return ray_depth - DepthThreshold <= z;
        }
    }
    return false;
}

float Rand1(inout float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

vec2 Rand2(inout float p) {
    return vec2(Rand1(p), Rand1(p));
}

float InitRand(vec2 uv) {
    vec3 p3  = fract(vec3(uv.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 SampleHemisphereUniform(inout float s, out float pdf) {
    vec2 uv = Rand2(s);
    float z = uv.x;
    float phi = uv.y * TWO_PI;
    float sinTheta = sqrt(1.0 - z*z);
    vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
    pdf = INV_TWO_PI;
    return dir;
}

vec3 SampleHemisphereCos(inout float s, out float pdf) {
    vec2 uv = Rand2(s);
    float z = sqrt(1.0 - uv.x);
    float phi = uv.y * TWO_PI;
    float sinTheta = sqrt(uv.x);
    vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
    pdf = z * INV_PI;
    return dir;
}

void LocalBasis(vec3 n, out vec3 b1, out vec3 b2) {
    float sign_ = sign(n.z);
    if (n.z == 0.0) {
        sign_ = 1.0;
    }
    float a = -1.0 / (sign_ + n.z);
    float b = n.x * n.y * a;
    b1 = vec3(1.0 + sign_ * n.x * n.x * a, sign_ * b, -sign_ * n.x);
    b2 = vec3(b, sign_ + n.y * n.y * a, -n.y);
}

vec4 Project(vec4 a) {
    return a / a.w;
}

float GetDepth(vec3 posWorld) {
    vec4 posView = View * vec4(posWorld, 1.0);
    //float depth = (View * vec4(posWorld, 1.0)).w;
    float depth = posView.z / posView.w;
    return -depth;
}

vec2 GetScreenCoordinate(vec3 posWorld) {
    vec2 uv = Project(Proj * View * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
    return uv;
}

float GetGBufferDepth(vec2 uv) {
    float depth = texture2D(GBuffer1, uv).z;
    if (depth < 1e-2) {
        depth = 1000.0;
    }
    return depth;
}

vec3 GetGBufferNormalWorld(vec2 uv) {
    // get from g-buffer
    vec4 p0 = texture(GBuffer0, uv);
    vec4 p1 = texture(GBuffer1, uv);
    vec2 oct_normal = vec2(p0.w, p1.w);
    vec3 normal = decodeNormal(oct_normal);
    return normal;
}

vec3 GetGBufferPosWorld(vec2 uv) {
    // get from g-buffer
    vec4 p0 = texture(GBuffer0, uv);
    vec3 posWorld = p0.xyz;

    return posWorld;
}

vec3 GetGBufferDiffuse(vec2 uv) {
    // get from g-buffer
    vec4 p1 = texture(GBuffer1, uv);
    vec2 color1 = unpackHalf2x16(floatBitsToUint(p1.x));
    vec3 albedo = vec3(color1.x, p1.g, color1.y);

    vec3 diffuse = pow(albedo, vec3(2.2));
    return diffuse;
}

float GetVisilibity(vec2 uv) {
    vec3 posWorld = GetGBufferPosWorld(uv);
    vec4 posLightSpace = lightSpaceMatrix * vec4(posWorld, 1.0);
    posLightSpace = posLightSpace / posLightSpace.w;
    float currentDepth = posLightSpace.z;

    vec2 lightUV = posLightSpace.xy * 0.5 + 0.5;

    float closetDepth = texture(DepthMap, lightUV).r;

    float visibility = 0.0;
    if (currentDepth - 0.005 < closetDepth)
    {
        visibility = 1.0;
    }

    return visibility;
}

float GetShadowFactor(vec2 uv)
{
    vec4 p0 = texture(GBuffer0, uv);
    vec4 p1 = texture(GBuffer1, uv);
    vec3 pos = p0.xyz;
    vec2 oct_normal = vec2(p0.w, p1.w);
    vec3 normal = decodeNormal(oct_normal);
    vec2 color1 = unpackHalf2x16(floatBitsToUint(p1.x));
    vec3 albedo = vec3(color1.x, p1.g, color1.y);

    //view_z : positive
    float view_z = p1.z;

    vec4 clip_coord = lightSpaceMatrix * vec4(pos + normal * 0.02, 1.0);
    vec3 ndc_coord = (clip_coord.xyz / clip_coord.w) * 0.5 + 0.5;
    float shadow_z = texture(DepthMap, ndc_coord.xy).r;
    float shadow_factor = shadow_z >= ndc_coord.z ? 1 : 0;

    return shadow_factor;
}

/*
 * Evaluate diffuse bsdf value.
 *
 * wi, wo are all in world space.
 * uv is in screen space, [0, 1] x [0, 1].
 * wi,wo入射方向和出射方向
 */
vec3 EvalDiffuse(vec3 wi, vec3 wo, vec2 uv) {
    // vec3 L = vec3(0.0);
    vec3 albedo = GetGBufferDiffuse(uv);//获取漫反射率
    vec3 normal =normalize(GetGBufferNormalWorld(uv));//获取法线
    float cosTheta =max(0.0,dot(normalize(wi),normal));//漫反射夹角不能为负! 不然间接光存在黑边
    return albedo *INV_PI *cosTheta;
}

vec3 EvalDirectionalLight(vec2 uv) {
    // vec3 Le = vec3(0.0);
    //vec3 Le = lightRadiance * GetVisilibity(uv);
    vec3 Le = lightRadiance * GetShadowFactor(uv);
    return Le;
}

/*
  RayMarch
*/
bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
    float step = 0.02;
    float halfStep = 0.025;
    vec3 iterPos = ori;

    for (int i = 0; i < 100; ++i)
    {
        vec3 nextPos = iterPos + dir * step;
        float nextDepth = GetDepth(nextPos);
        float closestDepth = GetGBufferDepth(GetScreenCoordinate(nextPos));
        if (nextDepth - closestDepth < M_EPS)
        {
            hitPos = nextPos;
            return true;
        }
        else
        {
            iterPos = nextPos;
        }
    }

    return false;
}

void main() {
    // get from g-buffer
    vec4 p0 = texture(GBuffer0, iUV);
    vec4 p1 = texture(GBuffer1, iUV);
    vec3 pos = p0.xyz;
    vec2 oct_normal = vec2(p0.w, p1.w);
    vec3 normal = decodeNormal(oct_normal);
    vec2 color1 = unpackHalf2x16(floatBitsToUint(p1.x));
    vec3 albedo = vec3(color1.x, p1.g, color1.y);

    vec3 wo = normalize(cameraPos - pos);
    vec3 wi = normalize(-lightDir);

    float s = InitRand(gl_FragCoord.xy);

    vec3 L_direct = EvalDiffuse(wi, wo,iUV) * EvalDirectionalLight(iUV);

    vec3 L_indirect = vec3(0.0);

    for (int i = 0; i < IndirectSampleCount; ++i)
    {
        float pdf = 0.0;
        Rand1(s);
        vec3 sampleDir = SampleHemisphereUniform(s, pdf);
        vec3 b1, b2;
        LocalBasis(normal, b1, b2);
        //TBN * sampleDir = Get sampleDir in WorldSpace
        sampleDir = normalize(mat3(b1, b2, normal) * sampleDir);
        vec3 hitPos = vec3(0.0);
        bool isHit = RayMarch(pos.xyz, sampleDir, hitPos);
        if (isHit)
        {
            vec2 local_uv = GetScreenCoordinate(hitPos);
            //vec3 local_wo = normalize(vPosWorld.xyz - hitPos);
            vec3 local_L = EvalDiffuse(sampleDir, wo, iUV) / pdf * EvalDiffuse(wi, sampleDir, local_uv) * EvalDirectionalLight(local_uv);
            L_indirect += local_L;
            //L_indirect += vec3(1.0);
        }
    }
    L_indirect = L_indirect / float(IndirectSampleCount);

    vec3 L = L_direct + L_indirect;
    vec3 color = pow(clamp(L, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2));
    oFragColor = vec4(vec3(color.rgb), 1.0);
}
