//TEST_IGNORE_FILE:
#version 460

#extension GL_NV_ray_tracing : require

#define TRACING_EPSILON 1e-6

layout(binding = 0) uniform texture2D samplerPosition_0;
layout(binding = 2) uniform sampler sampler_0;
layout(binding = 1) uniform texture2D samplerNormal_0;

struct Light_0
{
    vec4 position_0;
    vec4 color_0;
};

#define NUM_LIGHTS 17

layout(binding = 3)
layout(std140) uniform ubo_0
{
    Light_0 light_0;
    vec4 viewPos_0;
    layout(row_major) mat4x4 view_0;
    layout(row_major) mat4x4 model_0;
};

layout(binding = 5) uniform accelerationStructureNV as_0;

struct ShadowRay_0
{
    float hitDistance_0;
};
layout(location = 0) rayPayloadNV ShadowRay_0 p_0;

struct ReflectionRay_0
{
    float color_1;
};
layout(location = 1) rayPayloadNV ReflectionRay_0 p_1;

layout(rgba32f) layout(binding = 4) uniform image2D outputImage_0;

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void TraceRay_0(
    accelerationStructureNV AccelerationStructure_0,
    uint RayFlags_0,
    uint InstanceInclusionMask_0,
    uint RayContributionToHitGroupIndex_0,
    uint MultiplierForGeometryContributionToHitGroupIndex_0,
    uint MissShaderIndex_0,
    RayDesc_0 Ray_0,
    inout ShadowRay_0 Payload_0)
{
    p_0 = Payload_0;
    traceNV(
        AccelerationStructure_0,
        RayFlags_0,
        InstanceInclusionMask_0,
        RayContributionToHitGroupIndex_0,
        MultiplierForGeometryContributionToHitGroupIndex_0,
        MissShaderIndex_0,
        Ray_0.Origin_0,
        Ray_0.TMin_0,
        Ray_0.Direction_0,
        Ray_0.TMax_0,
        0);
    Payload_0 = p_0;
    return;
}

float saturate_0(float x_0)
{
    float _S1 = clamp(x_0, float(0), float(1));
    return _S1;
}

void TraceRay_1(
    accelerationStructureNV AccelerationStructure_1,
    uint RayFlags_1,
    uint InstanceInclusionMask_1,
    uint RayContributionToHitGroupIndex_1,
    uint MultiplierForGeometryContributionToHitGroupIndex_1,
    uint MissShaderIndex_1,
    RayDesc_0 Ray_1,
    inout ReflectionRay_0 Payload_1)
{
    p_1 = Payload_1;
    traceNV(
        AccelerationStructure_1,
        RayFlags_1,
        InstanceInclusionMask_1,
        RayContributionToHitGroupIndex_1,
        MultiplierForGeometryContributionToHitGroupIndex_1,
        MissShaderIndex_1,
        Ray_1.Origin_0,
        Ray_1.TMin_0,
        Ray_1.Direction_0,
        Ray_1.TMax_0,
        1);
    Payload_1 = p_1;
    return;
}

void main() 
{
    float atten_0;

    uvec3 _S2 = gl_LaunchIDNV;
    float _S3 = float(_S2.x) + 0.5;
    uvec3 _S4 = gl_LaunchSizeNV;
    float _S5 = _S3 / float(_S4.x);
    uvec3 _S6 = gl_LaunchIDNV;
    float _S7 = float(_S6.y) + 0.5;
    uvec3 _S8 = gl_LaunchSizeNV;
    float _S9 = _S7 / float(_S8.y);
    vec2 inUV_0 = vec2(_S5, _S9);
    
    vec4 _S10 = texture(sampler2D(samplerPosition_0, sampler_0), inUV_0);
    vec3 P_0 = _S10.xyz;

    vec4 _S11 = texture(sampler2D(samplerNormal_0, sampler_0), inUV_0);
    vec3 N_0 = _S11.xyz * 2.0 - 1.0;

    vec3 lightDelta_0 = light_0.position_0.xyz - P_0;
    float lightDist_0 = length(lightDelta_0);
    vec3 L_0 = normalize(lightDelta_0);

    float _S12 = 1.0 / (lightDist_0 * lightDist_0);

    RayDesc_0 ray_0;
    ray_0.Origin_0 = P_0;
    ray_0.TMin_0 = TRACING_EPSILON;
    ray_0.Direction_0 = lightDelta_0;
    ray_0.TMax_0 = lightDist_0;

    ShadowRay_0 shadowRay_0;
    shadowRay_0.hitDistance_0 = float(0);
    const uint _S13 = uint(1);
    const uint _S14 = uint(0xFF);
    const uint _S15 = uint(0);
    const uint _S16 = uint(0);
    const uint _S17 = uint(2);

    RayDesc_0 _S18 = ray_0;
    ShadowRay_0 _S19;
    _S19 = shadowRay_0;
    TraceRay_0(as_0, _S13, _S14, _S15, _S16, _S17, _S18, _S19);
    shadowRay_0 = _S19;

    bool _S20 = shadowRay_0.hitDistance_0 < lightDist_0;
    ReflectionRay_0 reflectionRay_0;
    if(_S20)
    {
        atten_0 = (0.00000000000000000000);
    }
    else
    {
        atten_0 = _S12;
    }

    vec3 _S21 = light_0.color_0.xyz;
    float _S22 = dot(N_0, L_0);
    float _S23 = saturate_0(_S22);
    vec3 color_2 = (_S21 * _S23) * atten_0;

    const uint _S24 = uint(1);
    const uint _S25 = uint(255);
    const uint _S26 = uint(0);
    const uint _S27 = uint(0);
    const uint _S28 = uint(2);
    RayDesc_0 _S29 = ray_0;
    ReflectionRay_0 _S30;
    _S30 = reflectionRay_0;
    TraceRay_1(as_0, _S24, _S25, _S26, _S27, _S28, _S29, _S30);

    vec3 color_3 = color_2 + _S30.color_1;

    uvec3 _S31 = gl_LaunchIDNV;
    imageStore(outputImage_0, ivec2(uvec2(ivec2(_S31.xy))), vec4(color_3, 1.0));
    return;
}
