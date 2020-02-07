//TEST_IGNORE_FILE:

struct SLANG_ParameterGroup_C_0
{
    uint index_0;
};

cbuffer C_0 : register(b0)
{
    SLANG_ParameterGroup_C_0 C_0;
}

struct S_0
{
    float4 f_0;
};

ConstantBuffer<S_0>        cb_0 [3] : register(b1);
StructuredBuffer<S_0>      sb1_0[4] : register(t0);
RWStructuredBuffer<float4> sb2_0[5] : register(u0);
ByteAddressBuffer          bb_0[6]  : register(t4);

float4 main() : SV_TARGET
{
    float4 _S1 = cb_0[C_0.index_0].f_0;

    S_0 _S2 = sb1_0[C_0.index_0][C_0.index_0];

    float4 _S3 = _S1 + _S2.f_0;
    float4 _S4 = _S3 + sb2_0[C_0.index_0][C_0.index_0];
    uint _S5 = bb_0[C_0.index_0].Load(
        (int) (C_0.index_0 * (uint) 4));

    return _S4 + (float4) _S5;
}
