//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
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
