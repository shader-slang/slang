#version 450
#extension GL_EXT_shader_atomic_float : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 10 "tests/slang-extension/atomic-float-byte-address-buffer-cross.slang"
layout(std430, binding = 1) buffer _S1 {
    float _data[];
} anotherBuffer_0;

#line 60 "hlsl"
layout(std430, binding = 0) buffer _S2 {
    float _data[];
} _S3;

#line 18 "tests/slang-extension/atomic-float-byte-address-buffer-cross.slang"
void RWByteAddressBuffer_InterlockedAddF32_0(uint _S4, float _S5, out float _S6)
{
    uint _S7 = _S4 / uint(4);
    float _S8 = (atomicAdd((((_S3)._data[(_S7)])), (_S5)));
    _S6 = _S8;
    return;
}

void RWByteAddressBuffer_InterlockedAddF32_1(uint _S9, float _S10)
{
    uint _S11 = _S9 / uint(4);
    float _S12 = (atomicAdd((((_S3)._data[(_S11)])), (_S10)));
    return;
}


#line 13
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;void main()
{
    uint tid_0 = gl_GlobalInvocationID.x;
    int idx_0 = int(tid_0 & uint(3) ^ tid_0 >> 2);

    float delta_0 = ((anotherBuffer_0)._data[(uint(idx_0 & 3))]);

#line 21
    float _S13;
    RWByteAddressBuffer_InterlockedAddF32_0(uint(idx_0 << 2), 1.00000000000000000000, _S13);
    RWByteAddressBuffer_InterlockedAddF32_1(uint(int(tid_0 >> 2) << 2), delta_0);

#line 13
    return;
}