//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#checkpointing-and-rematerialization
//META: doc_section_digest=906b4594373567f3ec31f082093e645ccdf78e1c2d5241292010bf45237c6a41
//META: purpose=Verifies bwd_diff over a [MaxIters]-bounded accumulating for-loop accumulates the per-iteration gradient into the input derivative.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-primal-hoist.cpp
//META: characterization-unverified=true
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of primal-value hoisting / checkpointing across a
// reverse-mode loop. The body runs 3 times adding x*x, so f = 3*x^2 and
// df/dx = 6*x; at x=2 the accumulated input derivative is 12. The loop
// requires [MaxIters] for the differentiable loop transform; CHECK pins
// the CPU value, exercising the primal-hoist pass that stores/replays the
// loop-carried primal state for the reverse pass. Tagged unverified:
// asserted against the compiler's own current CPU output.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

typedef DifferentialPair<float> dpfloat;

[Differentiable]
float f(float x)
{
    float r = 0.0;
    [MaxIters(3)]
    for (int i = 0; i < 3; i++)
        r = r + x * x;
    return r;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    dpfloat dp = dpfloat(2.0, 0.0);
    __bwd_diff(f)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

// CHECK: 12.0
