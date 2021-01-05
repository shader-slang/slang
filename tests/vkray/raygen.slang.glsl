//TEST_IGNORE_FILE:
#version 460

layout(row_major) uniform;

#if USE_NV_RT
#extension GL_NV_ray_tracing : require
#define accelerationStructureEXT accelerationStructureNV
#define callableDataInEXT callableDataInNV
#define gl_LaunchIDEXT gl_LaunchIDNV
#define hitAttributeEXT hitAttributeNV
#define ignoreIntersectionEXT ignoreIntersectionNV
#define rayPayloadInEXT rayPayloadInNV
#define terminateRayEXT terminateRayNV
#define traceRayEXT traceNV
#else
#extension GL_EXT_ray_tracing : require
#endif

#define TRACING_EPSILON 1e-6

#define tmp_ubo             _S1
#define tmp_saturate        _S2
#define tmp_launchID_x      _S3
#define tmp_add_x           _S4
#define tmp_launchSize_x    _S5
#define tmp_div_x           _S6
#define tmp_launchID_y      _S7
#define tmp_add_y           _S8
#define tmp_launchSize_y    _S9
#define tmp_div_y           _S10
#define tmp_tex_pos         _S11
#define tmp_tex_nrm         _S12
#define tmp_light_invDist   _S13
#define tmp_trace_A         _S14
#define tmp_trace_B         _S15
#define tmp_trace_C         _S16
#define tmp_trace_D         _S17
#define tmp_trace_E         _S18
#define tmp_trace_ray       _S19
#define tmp_trace_payload   _S20
#define tmp_color           _S21
#define tmp_dot             _S22
#define tmp_sat             _S23
#define tmp_trace2_A        _S24
#define tmp_trace2_B        _S25
#define tmp_trace2_C        _S26
#define tmp_trace2_D        _S27
#define tmp_trace2_E        _S28
#define tmp_trace2_ray      _S39
#define tmp_trace2_payload  _S30
#define tmp_storeIdx        _S31


layout(binding = 0) uniform texture2D samplerPosition_0;
layout(binding = 2) uniform sampler sampler_0;
layout(binding = 1) uniform texture2D samplerNormal_0;

struct Light_0
{
    vec4 position_0;
    vec4 color_0;
};

#define NUM_LIGHTS 17

struct Uniforms_0
{
    Light_0 light_0;
    vec4 viewPos_0;
    mat4x4 view_0;
    mat4x4 model_0;
};

layout(binding = 3)
layout(std140) uniform tmp_ubo
{
    Uniforms_0 _data;
} ubo_0;

layout(binding = 5) uniform accelerationStructureEXT as_0;

struct ShadowRay_0
{
    float hitDistance_0;
};
layout(location = 0) rayPayloadEXT ShadowRay_0 p_0;

struct ReflectionRay_0
{
    float color_1;
};
layout(location = 1) rayPayloadEXT ReflectionRay_0 p_1;

layout(rgba32f) layout(binding = 4) uniform image2D outputImage_0;

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void TraceRay_0(
    accelerationStructureEXT AccelerationStructure_0,
    uint RayFlags_0,
    uint InstanceInclusionMask_0,
    uint RayContributionToHitGroupIndex_0,
    uint MultiplierForGeometryContributionToHitGroupIndex_0,
    uint MissShaderIndex_0,
    RayDesc_0 Ray_0,
    inout ShadowRay_0 Payload_0)
{
    p_0 = Payload_0;
    traceRayEXT(
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

void TraceRay_1(
    accelerationStructureEXT AccelerationStructure_1,
    uint RayFlags_1,
    uint InstanceInclusionMask_1,
    uint RayContributionToHitGroupIndex_1,
    uint MultiplierForGeometryContributionToHitGroupIndex_1,
    uint MissShaderIndex_1,
    RayDesc_0 Ray_1,
    inout ReflectionRay_0 Payload_1)
{
    p_1 = Payload_1;
    traceRayEXT(
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

float saturate_0(float x_0)
{
    float tmp_saturate = clamp(x_0, float(0), float(1));
    return tmp_saturate;
}

void main() 
{
    float atten_0;

    uvec3 tmp_launchID_x = gl_LaunchIDEXT;
    float tmp_add_x = float(tmp_launchID_x.x) + 0.5;
    uvec3 tmp_launchSize_x = gl_LaunchSizeEXT;
    float tmp_div_x = tmp_add_x / float(tmp_launchSize_x.x);

    uvec3 tmp_launchID_y = gl_LaunchIDEXT;
    float tmp_add_y = float(tmp_launchID_y.y) + 0.5;
    uvec3 tmp_launchSize_y = gl_LaunchSizeEXT;
    float tmp_div_y = tmp_add_y / float(tmp_launchSize_y.y);
    vec2 inUV_0 = vec2(tmp_div_x, tmp_div_y);
    
    vec4 tmp_tex_pos = texture(sampler2D(samplerPosition_0, sampler_0), inUV_0);
    vec3 P_0 = tmp_tex_pos.xyz;

    vec4 tmp_tex_nrm = texture(sampler2D(samplerNormal_0, sampler_0), inUV_0);
    vec3 N_0 = tmp_tex_nrm.xyz * 2.0 - 1.0;

    vec3 lightDelta_0 = ubo_0._data.light_0.position_0.xyz - P_0;
    float lightDist_0 = length(lightDelta_0);
    vec3 L_0 = normalize(lightDelta_0);

    float tmp_light_invDist = 1.0 / (lightDist_0 * lightDist_0);

    RayDesc_0 ray_0;
    ray_0.Origin_0 = P_0;
    ray_0.TMin_0 = TRACING_EPSILON;
    ray_0.Direction_0 = lightDelta_0;
    ray_0.TMax_0 = lightDist_0;

    ShadowRay_0 shadowRay_0;
    shadowRay_0.hitDistance_0 = float(0);
    const uint tmp_trace_A = uint(1);
    const uint tmp_trace_B = uint(0xFF);
    const uint tmp_trace_C = uint(0);
    const uint tmp_trace_D = uint(0);
    const uint tmp_trace_E = uint(2);

    RayDesc_0 tmp_trace_ray = ray_0;
    ShadowRay_0 tmp_trace_payload;
    tmp_trace_payload = shadowRay_0;
    TraceRay_0(as_0, tmp_trace_A, tmp_trace_B, tmp_trace_C, tmp_trace_D, tmp_trace_E, tmp_trace_ray, tmp_trace_payload);
    shadowRay_0 = tmp_trace_payload;

    ReflectionRay_0 reflectionRay_0;
    if(shadowRay_0.hitDistance_0 < lightDist_0)
    {
        atten_0 = (0.00000000000000000000);
    }
    else
    {
        atten_0 = tmp_light_invDist;
    }

    vec3 tmp_color = ubo_0._data.light_0.color_0.xyz;
    float tmp_dot = dot(N_0, L_0);
    float tmp_sat = saturate_0(tmp_dot);
    vec3 color_2 = (tmp_color * tmp_sat) * atten_0;

    const uint tmp_trace2_A = uint(1);
    const uint tmp_trace2_B = uint(255);
    const uint tmp_trace2_C = uint(0);
    const uint tmp_trace2_D = uint(0);
    const uint tmp_trace2_E = uint(2);
    RayDesc_0 tmp_trace2_ray = ray_0;
    ReflectionRay_0 tmp_trace2_payload;
    tmp_trace2_payload = reflectionRay_0;
    TraceRay_1(as_0, tmp_trace2_A, tmp_trace2_B, tmp_trace2_C, tmp_trace2_D, tmp_trace2_E, tmp_trace2_ray, tmp_trace2_payload);

    vec3 color_3 = color_2 + tmp_trace2_payload.color_1;

    uvec3 tmp_storeIdx = gl_LaunchIDEXT;
    imageStore(outputImage_0, ivec2(uvec2(ivec2(tmp_storeIdx.xy))), vec4(color_3, 1.0));
    return;
}
