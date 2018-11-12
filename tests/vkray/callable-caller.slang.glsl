#version 460

layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_NV_ray_tracing : require

struct SLANG_ParameterGroup_C_0
{
    uint shaderIndex_0;
};

layout(binding = 0)
layout(std140) uniform C_0
{
    uint shaderIndex_0;
};

struct MaterialPayload_0
{
    vec4 albedo_0;
    vec2 uv_0;
};

layout(location = 0)
rayPayloadNV MaterialPayload_0 p_0;

layout(rgba32f)
layout(binding = 1)
uniform image2D gImage_0;

void CallShader_0(
    uint shaderIndex_1,
    inout MaterialPayload_0 payload_0)
{
    p_0 = payload_0;
    executeCallableNV(shaderIndex_1, (0));
    payload_0 = p_0;
    return;
}

void main()
{
    MaterialPayload_0 payload_1;
    payload_1.albedo_0 = vec4(0);

    uvec3 _S1 = gl_LaunchIDNV;
    vec2 _S2 = vec2(_S1.xy);

    uvec3 _S3 = gl_LaunchSizeNV;
    vec2 _S4 = _S2 / vec2(_S3.xy);

    payload_1.uv_0 = _S4;

    uint _S5 = shaderIndex_0;

    MaterialPayload_0 _S6;
    _S6 = payload_1;
    CallShader_0(_S5, _S6);
    payload_1 = _S6;

    uvec3 _S7 = gl_LaunchIDNV;
    imageStore(
        gImage_0,
        ivec2(_S7.xy),
        payload_1.albedo_0);
    return;
}
