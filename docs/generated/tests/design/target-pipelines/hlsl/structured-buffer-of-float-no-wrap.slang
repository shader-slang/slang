//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=5acb4ce614573d6b46a7a78f0ba6dac2fcbcebec94dcc7e7e1fc9c7cbb5bc1fd
//META: purpose=Boundary: a StructuredBuffer<float> with a scalar (non-matrix) element type does NOT trigger the wrapper-struct rewrite.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: paired counter-test to wrap-structured-buffer-of-matrix —
// `wrapStructuredBuffersOfMatrices` only fires when the element
// type is a matrix. A scalar `float` element passes through
// unwrapped.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<float> inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x] * 2.0;
}

// CHECK: StructuredBuffer<float{{.*}}> inBuf_{{[0-9]+}} : register(t{{[0-9]+}})
