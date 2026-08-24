//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=51a3cfba2e9b446866969702a281584b197e5b4c38e04278312f866f3a25794f
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

// Emission fan-out: the `ColMajor` / `RowMajor` token is SPIR-V's own spelling,
// but the layout choice behind it reaches every text target, so each one gets a
// row-major/column-major pair here too. HLSL and GLSL can only express it at
// the declaration (`#pragma pack_matrix(...)`, `layout(<order>) uniform;`) —
// their use sites are byte-identical between the two modes — while Metal and
// WGSL carry no layout qualifier at all and instead show the choice at the
// *use* site: row-major reads the matrix member directly, column-major
// reconstructs it element-by-element in transposed order. The Metal and WGSL
// directives below therefore pin the unpack order, which is a different
// emitter path from the declaration token pinned in
// `matrix-layout-row-major-hlsl-pragma.slang`.

//TEST:SIMPLE(filecheck=ROW):-target spirv-asm -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=COL):-target spirv-asm -matrix-layout-column-major -entry main -stage compute
//TEST:SIMPLE(filecheck=HLR):-target hlsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=HLC):-target hlsl -matrix-layout-column-major -entry main -stage compute
//TEST:SIMPLE(filecheck=GLR):-target glsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=GLC):-target glsl -matrix-layout-column-major -entry main -stage compute
//TEST:SIMPLE(filecheck=MTR):-target metal -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=MTC):-target metal -matrix-layout-column-major -entry main -stage compute
//TEST:SIMPLE(filecheck=WGR):-target wgsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=WGC):-target wgsl -matrix-layout-column-major -entry main -stage compute

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

//HLR: #pragma pack_matrix(row_major)
//HLC: #pragma pack_matrix(column_major)

//GLR: layout(column_major) uniform;
//GLC: layout(row_major) uniform;

//MTR: (float4(1.0f, 2.0f, 3.0f, 4.0f)) * (globalParams_{{[0-9]+}}->m_{{[0-9]+}})
//MTC: ->m_{{[0-9]+}}.data_{{[0-9]+}}[int(0)][int(0)], globalParams_{{[0-9]+}}->m_{{[0-9]+}}.data_{{[0-9]+}}[int(1)][int(0)]

//WGR: .m_{{[0-9]+}}.data_{{[0-9]+}}[i32(0)][i32(0)], globalParams_{{[0-9]+}}.m_{{[0-9]+}}.data_{{[0-9]+}}[i32(0)][i32(1)]
//WGC: .m_{{[0-9]+}}.data_{{[0-9]+}}[i32(0)][i32(0)], globalParams_{{[0-9]+}}.m_{{[0-9]+}}.data_{{[0-9]+}}[i32(1)][i32(0)]
