//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=9e9f064c6da87187f2564f01994893ebb0371637146be4019d7b0f9e4145ca01
//META: purpose=Verifies fwd_diff over x*x emits a forward-derivative function s_fwd_f and a DiffPair struct carrying primal/differential fields.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-fwd.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the forward-mode transcription pass: applying
// __fwd_diff to a [Differentiable] scalar function synthesises a new
// function whose emitted name is `s_fwd_<orig>` and whose parameter/return
// type is the differential-pair struct `DiffPair_float_*` with `primal_*`
// and `differential_*` fields. CHECK pins those structural tokens (the
// derivative VALUE is checked separately by the COMPARE_COMPUTE tests so
// this file only asserts emission shape, which is stable).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

[Differentiable]
float f(float x)
{
    return x * x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> dp = DifferentialPair<float>(3.0, 1.0);
    DifferentialPair<float> r = __fwd_diff(f)(dp);
    outputBuffer[0] = r.d;
}

// HLSL-DAG: struct DiffPair_float_0
// HLSL-DAG: float primal_0;
// HLSL-DAG: float differential_0;
// HLSL-DAG: DiffPair_float_0 s_fwd_f_0(DiffPair_float_0
// CUDA-DAG: struct DiffPair_float_0
// CUDA-DAG: s_fwd_f_0(DiffPair_float_0
