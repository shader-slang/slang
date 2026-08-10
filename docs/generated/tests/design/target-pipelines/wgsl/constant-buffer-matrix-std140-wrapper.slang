//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
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
