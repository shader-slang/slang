// vk-structured-buffer-binding.hlsl.glsl
//TEST_IGNORE_FILE:

#version 450

#define gDoneGroups gDoneGroups_0
#define uv _S3
#define SV_Target _S2

layout(std430, binding = 3, set = 4)
buffer _S1
{
    uint _data[];
} gDoneGroups;

layout(location = 0)
out vec4 SV_Target;

layout(location = 0)
in vec3 uv;

void main()
{
    SV_Target = vec4(gDoneGroups._data[uint(int(uv.z))]);
    return;
}
