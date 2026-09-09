//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=8041b8729de3f94a856ac55d2d8461ddffd491518349fcd47c2422a784e25a3a
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
