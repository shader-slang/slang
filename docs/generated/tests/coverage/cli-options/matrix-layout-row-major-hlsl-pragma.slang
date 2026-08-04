//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=d95eda883eb0804ab1b80bcfd1567b5546c822286eb81da20bfc48d4944ed849
//META: purpose=Verifies -matrix-layout-row-major makes HLSL emit `#pragma pack_matrix(row_major)` instead of the default column_major.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the -matrix-layout-{row,column}-major CLI option
// (OptionKind::MatrixLayoutRow / MatrixLayoutColumn in slang-options.cpp).
// The chosen default matrix layout is threaded into HLSL emit as a
// `#pragma pack_matrix(...)` directive. CHECK pins that -matrix-layout-row-major
// produces the `row_major` pragma; the second directive (no flag) confirms the
// emitter defaults to `column_major`.

//TEST:SIMPLE(filecheck=ROW):-target hlsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=COL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float4> outBuf;
uniform float4x4 m;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = mul(m, float4(1, 2, 3, 4));
}

//ROW: #pragma pack_matrix(row_major)
//COL: #pragma pack_matrix(column_major)
