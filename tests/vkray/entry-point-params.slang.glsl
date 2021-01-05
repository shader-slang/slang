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
buffer _S1 {
    float _data[];
} buffer_0;

struct EntryPointParams_0
{
    float value_0;
};

layout(shaderRecordEXT)
buffer _S2
{
    EntryPointParams_0 _data;
} _S3;

void main()
{
    uvec3 _S4 = gl_LaunchIDEXT;
    buffer_0._data[_S4.x] = _S3._data.value_0;
    return;
}
