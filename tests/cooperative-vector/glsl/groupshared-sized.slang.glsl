#version 450
#extension GL_NV_cooperative_vector : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 0) buffer StructuredBuffer_float_t_0 {
    float _data[];
} outputBuffer_0;
shared float  buf_0[100];

layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
void main()
{
    coopvecNV<float, 8 > r_0;
    coopvecNV<int, 16 > v_0;
    uint _S1 = gl_GlobalInvocationID.x;
    if(_S1 == 0U)
    {
        int i_0 = 0;
        for(;;)
        {
            if(i_0 < 100)
            {
            }
            else
            {
                break;
            }
            buf_0[i_0] = float(i_0);
            i_0 = i_0 + 1;
        }
    }
    coopVecLoadNV((v_0), (buf_0), (0U));
    coopvecNV<int, 16 > _S2 = v_0;
    coopVecMatMulNV((r_0), (_S2), (1), (buf_0), (0U), (1), (8U), (16U), (0), (false), (4U));
    float _S3 = r_0[0U];
    outputBuffer_0._data[uint(_S1)] = _S3;
    return;
}
