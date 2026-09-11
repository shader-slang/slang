//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-construction
//META: doc_section_digest=e2a5528190c393d3701cc8f2de7339874632f622a35e167abada2fa7bfd2e742
//META: purpose=Verifies bwd_diff over a function taking an IDifferentiable struct accumulates a per-field gradient into the struct's Differential.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-transpose.cpp
//META: characterization-unverified=true
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of reverse-mode over an aggregate differentiable input:
// f(s) = s.a * s.b, so df/d(s.a) = s.b and df/d(s.b) = s.a. With a=2, b=5
// and seed 1.0 the input pair's Differential carries a=5 and b=2. CHECK
// pins both field gradients from the CPU runtime, exercising the
// transpose pass's per-field derivative accumulation into the struct's
// auto-synthesised Differential type. Tagged unverified: asserted against
// the compiler's own current CPU output.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct S : IDifferentiable
{
    float a;
    float b;
}

[Differentiable]
float f(S s)
{
    return s.a * s.b;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S sv = {2.0, 5.0};
    DifferentialPair<S> dp = DifferentialPair<S>(sv, S(0.0, 0.0));
    __bwd_diff(f)(dp, 1.0);
    outputBuffer[0] = dp.d.a;
    outputBuffer[1] = dp.d.b;
}

// CHECK: 5.0
// CHECK: 2.0
