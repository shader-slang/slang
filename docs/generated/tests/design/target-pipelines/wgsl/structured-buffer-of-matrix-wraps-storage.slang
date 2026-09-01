//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e9f2e62d5a5d7efaa803abbf9f002d3308a6da2c7e2ea7013076a2b7c23eb4c4
//META: purpose=lowerBufferElementTypeToStorageType wraps a structured buffer's matrix element in a _MatrixStorage_ struct on WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// On WGSL the lowering policy wraps RWStructuredBuffer<float4x4>
// in a generated struct '_MatrixStorage_float4x4_ColMajorstd430_*'
// whose 'data_*' field is the matrix stored as an array<vec4<f32>>.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
RWStructuredBuffer<float4x4> matBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = matBuf[tid.x][0][0];
}

// CHECK: struct _MatrixStorage_float4x4
// CHECK: array<vec4<f32>
