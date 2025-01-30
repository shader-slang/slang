#version 450
#extension GL_NV_cooperative_vector : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 0) buffer _S1 {
    float _data[];
} outputBuffer_0;
layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
void main()
{
    const coopvecNV<float, 8 > v_0 = coopvecNV<float, 8 >(1.0);
    const coopvecNV<float, 8 > r_0 = v_0 + v_0;
    float _S2 = r_0[0U];
    ((outputBuffer_0)._data[(gl_GlobalInvocationID.x)]) = _S2;
    return;
}
