#version 450
#extension GL_NV_cooperative_vector : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 0) buffer _S1
{
    uint _data[];
} buf_0;
layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
void main()
{
    coopvecNV<float, 8 > _S2 = (coopvecNV<float, 8>((1.0)));
    coopvecNV<float, 16 > _S3 = (coopvecNV<float, 16>((2.0)));
    coopVecOuterProductAccumulateNV((_S2), (_S3), (buf_0)._data, (0U), (4U), (0), (1));
    return;
}

