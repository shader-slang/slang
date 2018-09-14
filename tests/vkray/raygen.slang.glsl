//TEST_IGNORE_FILE:
#version 460

#extension GL_NVX_raytracing : require

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

layout(binding = 5) uniform accelerationStructureNVX as_0;

struct ShadowRay_0
{
    float hitDistance_0;
};
layout(location = 0) rayPayloadNVX ShadowRay_0 p_0;

struct ReflectionRay_0
{
    float color_1;
};
layout(location = 1) rayPayloadNVX ReflectionRay_0 p_1;

layout(rgba32f) layout(binding = 4) uniform image2D outputImage_0;

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void TraceRay_0(
    accelerationStructureNVX AccelerationStructure_0,
    uint RayFlags_0,
    uint InstanceInclusionMask_0,
    uint RayContributionToHitGroupIndex_0,
    uint MultiplierForGeometryContributionToHitGroupIndex_0,
    uint MissShaderIndex_0,
    RayDesc_0 Ray_0,
    inout ShadowRay_0 Payload_0)
{
    p_0 = Payload_0;
    vec3 _S1    = Ray_0.Origin_0;
    float _S2   = Ray_0.TMin_0;
    vec3 _S3    = Ray_0.Direction_0;
    float _S4   = Ray_0.TMax_0;
    int _S5 = 0;
    traceNVX(
        AccelerationStructure_0,
        RayFlags_0,
        InstanceInclusionMask_0,
        RayContributionToHitGroupIndex_0,
        MultiplierForGeometryContributionToHitGroupIndex_0,
        MissShaderIndex_0,
        _S1,
        _S2,
        _S3,
        _S4,
        _S5);
    Payload_0 = p_0;
    return;
}

float saturate_0(float x_0)
{
    float _S6 = clamp(x_0, float(0), float(1));
    return _S6;
}

void TraceRay_1(
    accelerationStructureNVX AccelerationStructure_1,
    uint RayFlags_1,
    uint InstanceInclusionMask_1,
    uint RayContributionToHitGroupIndex_1,
    uint MultiplierForGeometryContributionToHitGroupIndex_1,
    uint MissShaderIndex_1,
    RayDesc_0 Ray_1,
    inout ReflectionRay_0 Payload_1)
{
    p_1 = Payload_1;
    vec3 _S7    = Ray_1.Origin_0;
    float _S8   = Ray_1.TMin_0;
    vec3 _S9    = Ray_1.Direction_0;
    float _S10  = Ray_1.TMax_0;
    int _S11 = 1;
    traceNVX(
        AccelerationStructure_1,
        RayFlags_1,
        InstanceInclusionMask_1,
        RayContributionToHitGroupIndex_1,
        MultiplierForGeometryContributionToHitGroupIndex_1,
        MissShaderIndex_1,
        _S7,
        _S8,
        _S9,
        _S10,
        _S11);
    Payload_1 = p_1;
    return;
}

void main() 
{
    float atten_0;

    uvec3 _S12 = uvec3(gl_LaunchIDNVX, 0);
    float _S13 = float(_S12.x) + 0.5;
    uvec3 _S14 = uvec3(gl_LaunchSizeNVX, 0);
    float _S15 = _S13 / float(_S14.x);
    uvec3 _S16 = uvec3(gl_LaunchIDNVX, 0);
    float _S17 = float(_S16.y) + 0.5;
    uvec3 _S18 = uvec3(gl_LaunchSizeNVX, 0);
    float _S19 = _S17 / float(_S18.y);
    vec2 inUV_0 = vec2(_S15, _S19);
    
    vec4 _S20 = texture(sampler2D(samplerPosition_0, sampler_0), inUV_0);
    vec3 P_0 = _S20.xyz;

    vec4 _S21 = texture(sampler2D(samplerNormal_0, sampler_0), inUV_0);
    vec3 N_0 = _S21.xyz * 2.0 - 1.0;

    vec3 lightDelta_0 = light_0.position_0.xyz - P_0;
    float lightDist_0 = length(lightDelta_0);
    vec3 L_0 = normalize(lightDelta_0);

    float _S22 = 1.0 / (lightDist_0 * lightDist_0);

    RayDesc_0 ray_0;
    ray_0.Origin_0 = P_0;
    ray_0.TMin_0 = TRACING_EPSILON;
    ray_0.Direction_0 = lightDelta_0;
    ray_0.TMax_0 = lightDist_0;

    ShadowRay_0 shadowRay_0;
    shadowRay_0.hitDistance_0 = float(0);
    const uint _S23 = uint(1);
    const uint _S24 = uint(0xFF);
    const uint _S25 = uint(0);
    const uint _S26 = uint(0);
    const uint _S27 = uint(2);

    RayDesc_0 _S28 = ray_0;
    ShadowRay_0 _S29;
    _S29 = shadowRay_0;
    TraceRay_0(as_0, _S23, _S24, _S25, _S26, _S27, _S28, _S29);
    shadowRay_0 = _S29;

    bool _S30 = shadowRay_0.hitDistance_0 < lightDist_0;
    ReflectionRay_0 reflectionRay_0;
    if(_S30)
    {
        atten_0 = (0.00000000000000000000);
    }
    else
    {
        atten_0 = _S22;
    }

    vec3 _S31 = light_0.color_0.xyz;
    float _S32 = dot(N_0, L_0);
    float _S33 = saturate_0(_S32);
    vec3 color_2 = (_S31 * _S33) * atten_0;

    const uint _S34 = uint(1);
    const uint _S35 = uint(255);
    const uint _S36 = uint(0);
    const uint _S37 = uint(0);
    const uint _S38 = uint(2);
    RayDesc_0 _S39 = ray_0;
    ReflectionRay_0 _S40;
    _S40 = reflectionRay_0;
    TraceRay_1(as_0, _S34, _S35, _S36, _S37, _S38, _S39, _S40);

    vec3 color_3 = color_2 + _S40.color_1;

    uvec3 _S41 = uvec3(gl_LaunchIDNVX, 0);
    imageStore(outputImage_0, ivec2(uvec2(ivec2(_S41.xy))), vec4(color_3, 1.0));
    return;
}
