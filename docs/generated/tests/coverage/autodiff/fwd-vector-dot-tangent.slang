//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=ff7339aef6c3ffedb9466f9df10a8adb3a5ae02fe094735a717bd998cd8bc62a
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
