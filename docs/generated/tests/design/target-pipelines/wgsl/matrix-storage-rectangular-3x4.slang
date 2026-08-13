//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e9f2e62d5a5d7efaa803abbf9f002d3308a6da2c7e2ea7013076a2b7c23eb4c4
//META: purpose=A rectangular RWStructuredBuffer<float3x4> wraps in _MatrixStorage_float3x4_ColMajorstd430 with a 4-element array<vec3<f32>>
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Non-square (rectangular) matrix axis of the matrix-storage
// wrapper: the column-major layout means a float3x4 stores as
// array<vec3<f32>, 4> inside the wrapper.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
RWStructuredBuffer<float3x4> matBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = matBuf[tid.x][0][0];
}

// CHECK: struct _MatrixStorage_float3x4
// CHECK: array<vec3<f32>
