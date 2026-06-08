//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#downstream-dxc-fxc
//META: doc_section_digest=924b157419520ebcec2e0e4024679821ed4db2d92a3d90fcd3a21f934950d4d7
//META: purpose=`-target hlsl` stops at HLSL text; no DXIL, no SPIR-V, no bytecode is emitted, only HLSL source.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Phase D diagram: `CodeGenTarget::HLSL` stops at the
// text artifact. DXC and fxc are invoked only for DXIL /
// DXBytecode targets. So `-target hlsl` output is plain HLSL
// (we observe HLSL-canonical tokens).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #pragma pack_matrix(column_major)
// CHECK: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}}
// CHECK: void main(
