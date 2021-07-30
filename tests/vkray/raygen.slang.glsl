//TEST_IGNORE_FILE:
#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(binding = 0)
uniform texture2D samplerPosition_0;

layout(binding = 2)
uniform sampler sampler_0;

layout(binding = 1)
uniform texture2D samplerNormal_0;

struct Light_0
{
    vec4 position_0;
    vec4 color_0;
};

struct Uniforms_0
{
    Light_0 light_0;
    vec4 viewPos_0;
    mat4x4 view_0;
    mat4x4 model_0;
};

layout(binding = 3)
layout(std140) uniform _S1
{
    Uniforms_0 _data;
} ubo_0;
struct ShadowRay_0
{
    float hitDistance_0;
};

layout(location = 0)
rayPayloadEXT
ShadowRay_0 p_0;

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void TraceRay_0(accelerationStructureEXT AccelerationStructure_0, uint RayFlags_0, uint InstanceInclusionMask_0, uint RayContributionToHitGroupIndex_0, uint MultiplierForGeometryContributionToHitGroupIndex_0, uint MissShaderIndex_0, RayDesc_0 Ray_0, inout ShadowRay_0 Payload_0)
{
    p_0 = Payload_0;
    traceRayEXT(AccelerationStructure_0, RayFlags_0, InstanceInclusionMask_0, RayContributionToHitGroupIndex_0, MultiplierForGeometryContributionToHitGroupIndex_0, MissShaderIndex_0, Ray_0.Origin_0, Ray_0.TMin_0, Ray_0.Direction_0, Ray_0.TMax_0, (0));
    Payload_0 = p_0;
    return;
}

struct ReflectionRay_0
{
    float color_1;
};

layout(location = 1)
rayPayloadEXT
ReflectionRay_0 p_1;

void TraceRay_1(accelerationStructureEXT AccelerationStructure_1, uint RayFlags_1, uint InstanceInclusionMask_1, uint RayContributionToHitGroupIndex_1, uint MultiplierForGeometryContributionToHitGroupIndex_1, uint MissShaderIndex_1, RayDesc_0 Ray_1, inout ReflectionRay_0 Payload_1)
{
    p_1 = Payload_1;
    traceRayEXT(AccelerationStructure_1, RayFlags_1, InstanceInclusionMask_1, RayContributionToHitGroupIndex_1, MultiplierForGeometryContributionToHitGroupIndex_1, MissShaderIndex_1, Ray_1.Origin_0, Ray_1.TMin_0, Ray_1.Direction_0, Ray_1.TMax_0, (1));
    Payload_1 = p_1;
    return;
}

layout(binding = 5)
uniform accelerationStructureEXT as_0;

float saturate_0(float x_0)
{
    float _S2 = clamp(x_0, float(0), float(1));
    return _S2;
}

layout(rgba32f)
layout(binding = 4)
uniform image2D outputImage_0;

void main()
{
    float atten_0;
    uvec3 _S3 = ((gl_LaunchIDEXT));
    float _S4 = float(_S3.x) + 0.50000000000000000000;
    uvec3 _S5 = ((gl_LaunchSizeEXT));
    float _S6 = _S4 / float(_S5.x);
    uvec3 _S7 = ((gl_LaunchIDEXT));
    float _S8 = float(_S7.y) + 0.50000000000000000000;
    uvec3 _S9 = ((gl_LaunchSizeEXT));
    float _S10 = _S8 / float(_S9.y);
    vec2 inUV_0 = vec2(_S6, _S10);
    vec4 _S11 = (texture(sampler2D(samplerPosition_0,sampler_0), (inUV_0)));
    vec3 P_0 = _S11.xyz;
    vec4 _S12 = (texture(sampler2D(samplerNormal_0,sampler_0), (inUV_0)));
    vec3 N_0 = _S12.xyz * 2.00000000000000000000 - 1.00000000000000000000;
    vec3 lightDelta_0 = ubo_0._data.light_0.position_0.xyz - P_0;
    float lightDist_0 = length(lightDelta_0);
    vec3 L_0 = normalize(lightDelta_0);
    float _S13 = 1.00000000000000000000 / (lightDist_0 * lightDist_0);
    RayDesc_0 ray_0;
    ray_0.Origin_0 = P_0;
    ray_0.TMin_0 = 0.00000100000000000000;
    ray_0.Direction_0 = lightDelta_0;
    ray_0.TMax_0 = lightDist_0;
    ShadowRay_0 shadowRay_0;
    shadowRay_0.hitDistance_0 = float(0);
    TraceRay_0(as_0, uint(1), uint(255), uint(0), uint(0), uint(2), ray_0, shadowRay_0);
    if(shadowRay_0.hitDistance_0 < lightDist_0)
    {
        atten_0 = 0.00000000000000000000;
    }
    else
    {
        atten_0 = _S13;
    }
    vec3 _S14 = ubo_0._data.light_0.color_0.xyz;
    float _S15 = dot(N_0, L_0);
    float _S16 = saturate_0(_S15);
    vec3 color_2 = _S14 * _S16 * atten_0;
    ReflectionRay_0 reflectionRay_0;
    TraceRay_1(as_0, uint(1), uint(255), uint(0), uint(0), uint(2), ray_0, reflectionRay_0);
    vec3 color_3 = color_2 + reflectionRay_0.color_1;
    uvec3 _S17 = ((gl_LaunchIDEXT));
    imageStore((outputImage_0), ivec2((uvec2(ivec2(_S17.xy)))), vec4(color_3, 1.00000000000000000000));
    return;
}
