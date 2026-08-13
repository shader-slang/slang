//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=9e9f064c6da87187f2564f01994893ebb0371637146be4019d7b0f9e4145ca01
//META: purpose=Verifies fwd_diff propagates the tangent through the taken branch of an if/else inside a differentiable function.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-fwd.cpp
//META: characterization-unverified=true
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of forward-mode transcription across control flow:
// the function returns x*x*x on the taken (x>0) branch, so at x=2 the
// primal is 8 and the forward derivative is 3*x^2 = 12. CHECK asserts the
// observed primal (8) and tangent (12) the CPU runtime produces, pinning
// that the fwd pass transcribes the branch the runtime actually executes.
// Tagged unverified: the derivative correctness is asserted only against
// the compiler's own current CPU output, not an independent oracle.

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

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    dpfloat dp = dpfloat(2.0, 1.0);
    dpfloat r = __fwd_diff(f)(dp);
    outputBuffer[0] = r.p;
    outputBuffer[1] = r.d;
}

// CHECK: 8.0
// CHECK: 12.0
