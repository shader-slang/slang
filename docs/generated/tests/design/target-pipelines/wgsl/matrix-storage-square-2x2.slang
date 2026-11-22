//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a15f36e90ce71c175813e4f9aaae1546ac333d5b71b1977e258855334d9377bf
//META: purpose=A small square RWStructuredBuffer<float2x2> wraps in _MatrixStorage_float2x2_ColMajorstd430 with @align(8)
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Smallest documented square matrix axis: float2x2 wraps with
// @align(8) (not 16) because each column is a vec2<f32>.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
RWStructuredBuffer<float2x2> matBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = matBuf[tid.x][0][0];
}

// CHECK: struct _MatrixStorage_float2x2
// CHECK: @align(8)
// CHECK-SAME: array<vec2<f32>
