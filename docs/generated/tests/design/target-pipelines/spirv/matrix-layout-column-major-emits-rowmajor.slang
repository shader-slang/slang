//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=51a3cfba2e9b446866969702a281584b197e5b4c38e04278312f866f3a25794f
//META: purpose=A column-major source matrix layout emits the transposed SPIR-V member decoration `RowMajor`, the opposite pole of the same transposition rule.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other pole of the layout transposition, so the pair proves
// the decoration tracks the requested layout rather than being a constant.
// Same shader as the row-major companion, compiled with the opposite layout
// flag; the CHECK pins RowMajor. Without both directions a single test
// could not distinguish "transposed" from "always emits one spelling".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -matrix-layout-column-major

struct M { float4x4 m; };
ConstantBuffer<M> cb;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(cb.m, float4(1, 2, 3, 4));
}

// CHECK-DAG: OpMemberDecorate {{%[a-zA-Z_0-9]+}} 0 RowMajor
// CHECK-DAG: OpMemberDecorate {{%[a-zA-Z_0-9]+}} 0 MatrixStride 16
