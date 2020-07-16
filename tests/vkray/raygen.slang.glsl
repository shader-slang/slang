#version 460
#extension GL_NV_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 "tests/vkray/raygen.slang"
layout(binding = 0)
uniform texture2D samplerPosition_0;


#line 7
layout(binding = 2)
uniform sampler sampler_0;


#line 6
layout(binding = 1)
uniform texture2D samplerNormal_0;

struct Light_0
{
    vec4 position_0;
    vec4 color_0;
};


#line 14
struct Uniforms_0
{
    Light_0 light_0;
    vec4 viewPos_0;
    mat4x4 view_0;
    mat4x4 model_0;
};


#line 21
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
rayPayloadNV
ShadowRay_0 p_0;


#line 34
struct ReflectionRay_0
{
    float color_1;
};

layout(location = 1)
rayPayloadNV
ReflectionRay_0 p_1;


#line 3698 "hlsl.meta.slang"
struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void TraceRay_0(accelerationStructureNV AccelerationStructure_0, uint RayFlags_0, uint InstanceInclusionMask_0, uint RayContributionToHitGroupIndex_0, uint MultiplierForGeometryContributionToHitGroupIndex_0, uint MissShaderIndex_0, RayDesc_0 Ray_0, inout ShadowRay_0 Payload_0)
{
    p_0 = Payload_0;
    traceNV(AccelerationStructure_0, RayFlags_0, InstanceInclusionMask_0, RayContributionToHitGroupIndex_0, MultiplierForGeometryContributionToHitGroupIndex_0, MissShaderIndex_0, Ray_0.Origin_0, Ray_0.TMin_0, Ray_0.Direction_0, Ray_0.TMax_0, (0));
    Payload_0 = p_0;
    return;
}

void TraceRay_1(accelerationStructureNV AccelerationStructure_1, uint RayFlags_1, uint InstanceInclusionMask_1, uint RayContributionToHitGroupIndex_1, uint MultiplierForGeometryContributionToHitGroupIndex_1, uint MissShaderIndex_1, RayDesc_0 Ray_1, inout ReflectionRay_0 Payload_1)
{
    p_1 = Payload_1;
    traceNV(AccelerationStructure_1, RayFlags_1, InstanceInclusionMask_1, RayContributionToHitGroupIndex_1, MultiplierForGeometryContributionToHitGroupIndex_1, MissShaderIndex_1, Ray_1.Origin_0, Ray_1.TMin_0, Ray_1.Direction_0, Ray_1.TMax_0, (1));
    Payload_1 = p_1;
    return;
}


#line 27 "tests/vkray/raygen.slang"
layout(binding = 5)
uniform accelerationStructureNV as_0;

float saturate_0(float x_0)
{
    float _S2 = clamp(x_0, float(0), float(1));
    return _S2;
}


#line 25
layout(rgba32f)
layout(binding = 4)
uniform image2D outputImage_0;


#line 42
void main()
{
    float atten_0;

#line 39
    uvec3 _S3 = ((gl_LaunchIDNV));

#line 45
    float _S4 = float(_S3.x) + 0.50000000000000000000;

#line 40
    uvec3 _S5 = ((gl_LaunchSizeNV));

#line 45
    float _S6 = _S4 / float(_S5.x);

#line 39
    uvec3 _S7 = ((gl_LaunchIDNV));

#line 46
    float _S8 = float(_S7.y) + 0.50000000000000000000;

#line 40
    uvec3 _S9 = ((gl_LaunchSizeNV));

#line 46
    float _S10 = _S8 / float(_S9.y);

#line 44
    vec2 inUV_0 = vec2(_S6, _S10);

#line 49
    vec4 _S11 = (texture(sampler2D(samplerPosition_0,sampler_0), (inUV_0)));

#line 49
    vec3 P_0 = _S11.xyz;
    vec4 _S12 = (texture(sampler2D(samplerNormal_0,sampler_0), (inUV_0)));

#line 50
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



    const uint _S14 = uint(1);

    const uint _S15 = uint(255);

    const uint _S16 = uint(0);

    const uint _S17 = uint(0);

    const uint _S18 = uint(2);

#line 68
    RayDesc_0 _S19 = ray_0;

#line 68
    ShadowRay_0 _S20;

#line 68
    _S20 = shadowRay_0;

#line 68
    TraceRay_0(as_0, _S14, _S15, _S16, _S17, _S18, _S19, _S20);

#line 68
    shadowRay_0 = _S20;
    ReflectionRay_0 reflectionRay_0;

#line 84
    if(shadowRay_0.hitDistance_0 < lightDist_0)
    {
        atten_0 = 0.00000000000000000000;
    }
    else
    {
        atten_0 = _S13;
    }

#line 90
    vec3 _S21 = ubo_0._data.light_0.color_0.xyz;

#line 90
    float _S22 = dot(N_0, L_0);

#line 90
    float _S23 = saturate_0(_S22);

#line 90
    vec3 color_2 = _S21 * _S23 * atten_0;

#line 97
    const uint _S24 = uint(1);

    const uint _S25 = uint(255);

    const uint _S26 = uint(0);

    const uint _S27 = uint(0);

    const uint _S28 = uint(2);

#line 95
    RayDesc_0 _S29 = ray_0;

#line 95
    ReflectionRay_0 _S30;

#line 95
    _S30 = reflectionRay_0;

#line 95
    TraceRay_1(as_0, _S24, _S25, _S26, _S27, _S28, _S29, _S30);

#line 112
    vec3 color_3 = color_2 + _S30.color_1;

#line 39
    uvec3 _S31 = ((gl_LaunchIDNV));
imageStore(
#line 115
    (outputImage_0), ivec2((uvec2(ivec2(_S31.xy)))), vec4(color_3, 1.00000000000000000000));

#line 42
    return;
}

