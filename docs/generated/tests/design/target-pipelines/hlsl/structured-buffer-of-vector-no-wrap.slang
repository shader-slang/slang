//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
//META: purpose=Boundary: an `RWStructuredBuffer<float4>` element is a vector — NOT a matrix — so wrapStructuredBuffersOfMatrices does not wrap it.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: vector element vs matrix element. The pass only
// wraps when the element type is a matrix; vector elements ride
// through unchanged.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float4(1.0, 2.0, 3.0, 4.0);
}

// CHECK: RWStructuredBuffer<float4{{.*}}> outBuf_{{[0-9]+}}
// CHECK-NOT: _S
