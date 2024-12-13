#pragma pack_matrix(row_major)
RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};

struct A_0
{
    float x_0;
};

struct DiffPair_A_0
{
    A_0 primal_0;
    A_0 differential_0;
};

DiffPair_float_0 s_fwd_A_getVal_0(DiffPair_A_0 dpthis_0)
{
    DiffPair_float_0 _S1 = { dpthis_0.primal_0.x_0, dpthis_0.differential_0.x_0 };
    return _S1;
}

DiffPair_float_0 s_fwd_f_0(DiffPair_float_0 _S2, A_0 _S3)
{
    float _S4 = _S2.primal_0 * _S2.primal_0;
    float _S5 = _S2.differential_0 * _S2.primal_0;
    float _S6 = _S5 + _S5;
    DiffPair_float_0 _S7 = s_fwd_A_getVal_0(_S3);
    DiffPair_float_0 _S8 = { _S4 + _S7.primal_0, _S6 + _S7.differential_0 };
    return _S8;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    A_0 a_0;
    a_0.x_0 = 2.0;
    DiffPair_float_0 _S9 = { 1.5, 1.0 };
    DiffPair_float_0 _S10 = s_fwd_f_0(_S9, a_0);
    outputBuffer_0[int(0)] = _S10.primal_0;
    outputBuffer_0[int(1)] = _S10.differential_0;
    return;
}

