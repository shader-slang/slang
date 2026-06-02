//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a07249d171e8760079dd021f485b191eeba9855fbe96652d35a7e74a16b7586a
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
