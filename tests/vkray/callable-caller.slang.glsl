#version 460
#extension GL_NV_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;
struct MaterialPayload_0
{
    vec4 albedo_0;
    vec2 uv_0;
};

layout(location = 0)
callableDataNV
MaterialPayload_0 p_0;

struct SLANG_ParameterGroup_C_0
{
    uint shaderIndex_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;
void CallShader_0(uint shaderIndex_1, inout MaterialPayload_0 payload_0)
{
    p_0 = payload_0;
    int _S2 = (0);
    executeCallableNV(shaderIndex_1, _S2);
    payload_0 = p_0;
    return;
}

layout(rgba32f)
layout(binding = 1)
uniform image2D gImage_0;

void main()
{
    MaterialPayload_0 payload_1;
    payload_1.albedo_0 = vec4(0.0);
    uvec3 _S3 = ((gl_LaunchIDNV));
    vec2 _S4 = vec2(_S3.xy);
    uvec3 _S5 = ((gl_LaunchSizeNV));
    vec2 _S6 = _S4 / vec2(_S5.xy);
    payload_1.uv_0 = _S6;
    CallShader_0(C_0._data.shaderIndex_0, payload_1);
    uvec3 _S7 = ((gl_LaunchIDNV));
    imageStore((gImage_0), ivec2((_S7.xy)), payload_1.albedo_0);
    return;
}
