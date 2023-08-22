//TEST_IGNORE_FILE:
#version 460

#if USE_NV_RT
#extension GL_NV_ray_tracing : require
#define callableDataInEXT callableDataInNV
#define gl_LaunchIDEXT gl_LaunchIDNV
#define hitAttributeEXT hitAttributeNV
#define ignoreIntersectionEXT ignoreIntersectionNV
#define rayPayloadInEXT rayPayloadInNV
#define shaderRecordEXT shaderRecordNV
#define terminateRayEXT terminateRayNV
#else
#extension GL_EXT_ray_tracing : require
#endif

layout(std430, binding = 0)
buffer StructuredBuffer_float_t_0 {
    float _data[];
} buffer_0;

struct EntryPointParams_0
{
    float value_0;
};

layout(shaderRecordEXT)
buffer _S1
{
    float value_0;
} _S2;

void main()
{
    uvec3 _S3 = gl_LaunchIDEXT;
    buffer_0._data[_S3.x] = _S2.value_0;
    return;
}
