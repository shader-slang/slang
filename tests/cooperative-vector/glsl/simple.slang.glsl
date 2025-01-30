#version 450
#extension GL_NV_cooperative_vector : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 1) readonly buffer StructuredBuffer_float_t_0 {
    float _data[];
} buf_0;
layout(std430, binding = 0) buffer StructuredBuffer_float_t_1 {
    float _data[];
} outputBuffer_0;
layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
void main()
{
    coopvecNV<int, 16 > _S1 = (coopvecNV<int, 16>((1)));
    coopvecNV<int, 8 > r_0;
    coopVecMatMulNV((r_0), (_S1), (1), (buf_0)._data, (0U), (1), (8U), (16U), (0), (false), (4U));
    float _S2 = r_0[0U];
    outputBuffer_0._data[gl_GlobalInvocationID.x] = _S2;
    return;
}
