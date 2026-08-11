//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#checkpointing-and-rematerialization
//META: doc_section_digest=906b4594373567f3ec31f082093e645ccdf78e1c2d5241292010bf45237c6a41
//META: purpose=Verifies bwd_diff over a bounded loop emits a backward-propagation function plus a callable-context struct that carries the hoisted primal state.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-primal-hoist.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the primal-hoist + transpose interaction for a
// reverse-mode loop: the emitted HLSL contains the propagation function
// `s_bwdProp_<orig>`, a callable-context struct `s_bwdCallableCtx_<orig>`
// holding the captured loop state, and a re-emitted loop (`for(;;)`) in
// the reverse pass. CHECK pins those structural tokens; this is the
// emission counterpart to the value-level loop gradient test.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

[Differentiable]
float poly(float x)
{
    float r = 0.0;
    [MaxIters(3)]
    for (int i = 0; i < 3; i++)
        r = r + x * x;
    return r;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> dp = DifferentialPair<float>(2.0, 0.0);
    __bwd_diff(poly)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

// HLSL-DAG: struct s_bwdCallableCtx_poly_0
// HLSL-DAG: void s_bwdProp_poly_0(
// HLSL-DAG: for(;;)
