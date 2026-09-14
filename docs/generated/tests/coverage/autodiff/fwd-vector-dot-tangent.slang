//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=9e9f064c6da87187f2564f01994893ebb0371637146be4019d7b0f9e4145ca01
//META: purpose=Verifies fwd_diff propagates a float3 tangent through dot(v,v), producing primal and directional-derivative scalars.
//META: intent=characterization
//META: covers=source/slang/slang-ir-autodiff-fwd.cpp
//META: characterization-unverified=true
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of forward-mode over a vector input: f(v) = dot(v,v),
// whose directional derivative along tangent t is 2*dot(v,t). With
// v=(1,2,3) and t=(1,1,1) the primal is 14 and the tangent is 12. CHECK
// pins the CPU values, exercising the fwd pass's handling of a
// differential-pair-of-vector input and the dot intrinsic. Tagged
// unverified: asserted against the compiler's own current CPU output.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

typedef DifferentialPair<float3> dpv;

[Differentiable]
float f(float3 v)
{
    return dot(v, v);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    dpv dp = dpv(float3(1, 2, 3), float3(1, 1, 1));
    DifferentialPair<float> r = __fwd_diff(f)(dp);
    outputBuffer[0] = r.p;
    outputBuffer[1] = r.d;
}

// CHECK: 14.0
// CHECK: 12.0
