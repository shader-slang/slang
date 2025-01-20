#version 450
#extension GL_NV_cooperative_vector : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 0) buffer StructuredBuffer_float_t_0 {
    float _data[];
} rwBuf_0;
layout(std430, binding = 1) readonly buffer StructuredBuffer_float_t_1 {
    float _data[];
} buf_0;
layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
void main()
{
    coopvecNV<float, 8 > r_0;
    coopVecLoadNV((r_0), (rwBuf_0)._data, (0U));
    coopVecLoadNV((r_0), (buf_0)._data, (1U));
    coopVecStoreNV((r_0), (rwBuf_0)._data, (2U));
    return;
}

