// vk-structured-buffer-binding.hlsl.glsl
//TEST_IGNORE_FILE:

#version 450
layout(row_major) uniform;
layout(row_major) buffer;

layout(std430, binding = 3, set = 4) buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} gDoneGroups_0;

layout(location = 0)
out vec4 main_0;

layout(location = 0)
in vec3 uv_0;

void main()
{
    main_0 = vec4(float(gDoneGroups_0._data[uint(int(uv_0.z))]));
    return;
}


