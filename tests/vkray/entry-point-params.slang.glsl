//TEST_IGNORE_FILE:
#version 460
#extension GL_NV_ray_tracing : require

layout(std430, binding = 0)
buffer _S1 {
    float _data[];
} buffer_0;

struct EntryPointParams_0
{
    float value_0;
};

layout(shaderRecordNV)
buffer _S2
{
    EntryPointParams_0 _data;
} _S3;

void main()
{
    uvec3 _S4 = gl_LaunchIDNV;
    buffer_0._data[_S4.x] = _S3._data.value_0;
    return;
}
