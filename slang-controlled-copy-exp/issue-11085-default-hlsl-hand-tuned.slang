// Hand-tuned Slang translation of the default synthesized AD HLSL shape
// from issue #11085.
//
// Changes from generated HLSL:
// - use Slang source syntax;
// - mark AD-style helper functions ForceInline;
// - use inout/__constref parameters instead of value/out parameters where useful;
// - replace unrolled 128-lane generated code with ordinary loops;
// - load/store all lanes from buffers to keep the aggregate live.
//
RWStructuredBuffer<float> outputBuffer;
StructuredBuffer<float> inputBuffer;

struct BigVec_0
{
    float data_0[128];
};

struct s_bwdCallableCtx_inner_0
{
    BigVec_0 _S1;
};

struct DiffPair_BigVec_0
{
    BigVec_0 primal_0;
    BigVec_0 differential_0;
};

[ForceInline]
void innerBwd_0(inout DiffPair_BigVec_0 input_0, __constref BigVec_0 dResult_0)
{
    BigVec_0 dInput_0;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        dInput_0.data_0[i] = 2.0f * input_0.primal_0.data_0[i] * dResult_0.data_0[i];

    input_0.primal_0 = input_0.primal_0;
    input_0.differential_0 = dInput_0;
}

struct s_paramCtx_s_bwdCallableCtx_outer_0
{
    BigVec_0 value0_0;
};

struct s_bwdCallableCtx_outer_0
{
    s_paramCtx_s_bwdCallableCtx_outer_0 value0_1;
};

[ForceInline]
BigVec_0 BigVec_x24_syn_dzero_0()
{
    BigVec_0 result_0;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        result_0.data_0[i] = 0.0f;
    return result_0;
}

[ForceInline]
BigVec_0 BigVec_x24_syn_dadd_0(__constref BigVec_0 a, __constref BigVec_0 b)
{
    BigVec_0 result_0;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        result_0.data_0[i] = a.data_0[i] + b.data_0[i];
    return result_0;
}

[ForceInline]
void s_bwdProp_inner_0(
    inout s_bwdCallableCtx_inner_0 ctx_0,
    inout BigVec_0 d_input_0,
    __constref BigVec_0 d_out_0)
{
    DiffPair_BigVec_0 _S2;
    _S2.primal_0 = ctx_0._S1;
    innerBwd_0(_S2, d_out_0);
    d_input_0 = _S2.differential_0;
}

[ForceInline]
void s_bwdProp_outer_0(
    inout s_bwdCallableCtx_outer_0 _S3,
    inout BigVec_0 _S4,
    __constref BigVec_0 _S5)
{
    s_bwdCallableCtx_inner_0 _S6;
    _S6._S1 = _S3.value0_1.value0_0;

    BigVec_0 _S7 = BigVec_x24_syn_dzero_0();
    BigVec_0 _S9;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        _S9.data_0[i] = 0.0f;

    s_bwdProp_inner_0(_S6, _S9, _S5);
    _S4 = BigVec_x24_syn_dadd_0(_S9, _S7);
}

[ForceInline]
void _S10(inout DiffPair_BigVec_0 _S11, __constref BigVec_0 _S12)
{
    s_paramCtx_s_bwdCallableCtx_outer_0 _S13;
    _S13.value0_0 = _S11.primal_0;

    s_bwdCallableCtx_outer_0 _S14;
    _S14.value0_1 = _S13;

    s_bwdProp_outer_0(_S14, _S11.differential_0, _S12);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    BigVec_0 input_1;
    BigVec_0 dOutput_0;

    [ForceUnroll]
    for (int i = 0; i < 128; i++)
    {
        input_1.data_0[i] = inputBuffer[i];
        dOutput_0.data_0[i] = inputBuffer[128 + i];
    }

    BigVec_0 _S15 = BigVec_x24_syn_dzero_0();
    DiffPair_BigVec_0 inputPair_0;
    inputPair_0.primal_0 = input_1;
    inputPair_0.differential_0 = _S15;

    _S10(inputPair_0, dOutput_0);

    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        outputBuffer[i] = inputPair_0.differential_0.data_0[i];
}
