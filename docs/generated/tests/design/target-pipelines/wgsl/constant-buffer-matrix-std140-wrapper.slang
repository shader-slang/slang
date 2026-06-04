//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
//META: purpose=A ConstantBuffer holding a matrix uses the std140 (not std430) flavour of the matrix wrapper struct
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge of the matrix-storage axis: in a uniform-buffer context
// (ConstantBuffer) the wrapper is spelled
// _MatrixStorage_float4x4_ColMajorstd140 (std140) rather than the
// std430 form used for RWStructuredBuffer.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

struct CB { float4x4 m; };
ConstantBuffer<CB> cb;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = cb.m[0][0];
}

// CHECK: struct _MatrixStorage_float4x4_ColMajorstd140
// CHECK: var<uniform>
