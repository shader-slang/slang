#version 460

layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_NV_ray_tracing : require

#define tmp_ubo         _S1
#define tmp_launchid    _S2
#define tmp_luanchidf   _S3
#define tmp_launchsize  _S4
#define tmp_launchpos   _S5
#define tmp_shaderidx   _S6
#define tmp_payload     _S7
#define tmp_launchid2   _S8

struct SLANG_ParameterGroup_C_0
{
    uint shaderIndex_0;
};

layout(binding = 0)
layout(std140)
uniform tmp_ubo
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;

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

    uvec3 tmp_launchid = gl_LaunchIDNV;
    vec2 tmp_luanchidf = vec2(tmp_launchid.xy);

    uvec3 tmp_launchsize = gl_LaunchSizeNV;
    vec2 tmp_launchpos = tmp_luanchidf / vec2(tmp_launchsize.xy);

    payload_1.uv_0 = tmp_launchpos;

    uint tmp_shaderidx = C_0._data.shaderIndex_0;

    MaterialPayload_0 tmp_payload;
    tmp_payload = payload_1;
    CallShader_0(tmp_shaderidx, tmp_payload);
    payload_1 = tmp_payload;

    uvec3 tmp_launchid2 = gl_LaunchIDNV;
    imageStore(
        gImage_0,
        ivec2(tmp_launchid2.xy),
        payload_1.albedo_0);
    return;
}
