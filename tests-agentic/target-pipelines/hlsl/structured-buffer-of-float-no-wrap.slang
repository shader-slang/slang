//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
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
