//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=5acb4ce614573d6b46a7a78f0ba6dac2fcbcebec94dcc7e7e1fc9c7cbb5bc1fd
//META: purpose=Boundary: a 2x2 matrix element type in a structured buffer is still wrapped into a single-field struct, distinct from the 4x4 baseline.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: smallest documented matrix dim (2x2). The
// wrapStructuredBuffersOfMatrices pass must wrap any matrix
// element regardless of dim, so the 2x2 case still appears as
// `struct _S<N> { float2x2 ... }` in HLSL emit.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float2x2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float2x2(1.0, 2.0, 3.0, 4.0);
}

// CHECK: struct _S{{[0-9]+}}
// CHECK: float2x2 _S{{[0-9]+}};
// CHECK: RWStructuredBuffer<_S{{[0-9]+}}{{.*}}> outBuf_{{[0-9]+}}
