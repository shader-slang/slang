//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#reverse-mode
//META: doc_section_digest=b30cf23342891bc8437eb7a877a66ba2f5425da67381c5828e9114f9f947e7fb
//META: purpose=Verifies bwd_diff over x*x emits a backward-propagation function s_bwdProp and a callable-context struct s_bwdCallableCtx.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-transpose.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the reverse-mode transpose pass: applying
// __bwd_diff splits the function into a primal/propagate pair. The emitted
// HLSL names the propagation function `s_bwdProp_<orig>` and packages the
// captured primal state in a callable-context struct
// `s_bwdCallableCtx_<orig>`. CHECK pins those structural tokens, which are
// produced by the transpose pass independent of any backend value.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

[Differentiable]
float f(float x)
{
    return x * x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> dp = DifferentialPair<float>(3.0, 0.0);
    __bwd_diff(f)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

// HLSL-DAG: struct s_bwdCallableCtx_f_0
// HLSL-DAG: void s_bwdProp_f_0(
// HLSL-DAG: inout DiffPair_float_0
