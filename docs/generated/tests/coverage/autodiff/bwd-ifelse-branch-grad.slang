//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#reverse-mode
//META: doc_section_digest=b30cf23342891bc8437eb7a877a66ba2f5425da67381c5828e9114f9f947e7fb
//META: purpose=Verifies bwd_diff propagates the seed gradient through the taken branch of an if/else, accumulating into the input pair derivative.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-transpose.cpp
//META: characterization-unverified=true
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of reverse-mode transpose across an if/else: the
// taken branch (x>0) returns x*x*x, whose gradient w.r.t. x is 3*x^2.
// With seed 1.0 at x=2 the accumulated input derivative is 12. CHECK
// pins the CPU runtime value, exercising the transpose pass's branch
// reversal. Tagged unverified: asserted against the compiler's own
// current CPU output, not an external oracle.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

typedef DifferentialPair<float> dpfloat;

[Differentiable]
float f(float x)
{
    if (x > 0.0)
        return x * x * x;
    else
        return x * x;
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
