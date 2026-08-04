//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=730d07f4cad01d2edbbbf03523fae836c3254cc356ffd1ac578d0a21bad19381
//META: purpose=Verifies -matrix-layout-row-major emits the inverted `ColMajor` SPIR-V member decoration while -matrix-layout-column-major emits `RowMajor`.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the -matrix-layout-{row,column}-major option as observed
// in SPIR-V. Slang's matrix convention is transposed relative to SPIR-V's
// MatrixStride/Col/Row decoration, so the source-level layout selected on the
// CLI emits the *opposite* SPIR-V decoration token: -matrix-layout-row-major
// emits `ColMajor`, -matrix-layout-column-major emits `RowMajor`. This is the
// inversion a doc-only reader would not predict; the test pins it as the
// observed current behaviour (correctness vs the SPIR-V layout contract is not
// independently confirmed here).
//META: characterization-unverified=true

//TEST:SIMPLE(filecheck=ROW):-target spirv-asm -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=COL):-target spirv-asm -matrix-layout-column-major -entry main -stage compute

RWStructuredBuffer<float4> outBuf;
uniform float4x4 m;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = mul(m, float4(1, 2, 3, 4));
}

//ROW: OpMemberDecorate {{.*}} 0 ColMajor
//COL: OpMemberDecorate {{.*}} 0 RowMajor
