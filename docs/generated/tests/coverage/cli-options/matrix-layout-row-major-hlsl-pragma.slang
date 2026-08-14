//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=65ae1152b296463b8293b7591a32f87fe87ce08c5528a74ea4488483c5c3c5b7
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

// Emission fan-out: `#pragma pack_matrix` is the HLSL spelling of a layout
// decision every text target has to render somehow, so each target gets its own
// flag/default pair pinning its own spelling of the same delta. GLSL uses a
// module-scope `layout(<order>) uniform;` qualifier; SPIR-V uses a
// Col/RowMajor member decoration; Metal and WGSL have no layout qualifier at
// all and instead legalize the matrix into a `_MatrixStorage_*` struct whose
// name carries the order. Note that GLSL and SPIR-V both emit the *transposed*
// token (source row-major -> `column_major` / `ColMajor`), the same inversion
// documented on `matrix-layout-spirv-decoration.slang`.

//TEST:SIMPLE(filecheck=ROW):-target hlsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=COL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLR):-target glsl -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=GLC):-target glsl -matrix-layout-column-major -entry main -stage compute
//TEST:SIMPLE(filecheck=SPR):-target spirv-asm -matrix-layout-row-major -entry main -stage compute
//TEST:SIMPLE(filecheck=SPC):-target spirv-asm -matrix-layout-column-major -entry main -stage compute
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

//ROW: #pragma pack_matrix(row_major)
//COL: #pragma pack_matrix(column_major)

//GLR: layout(column_major) uniform;
//GLC: layout(row_major) uniform;

//SPR: OpMemberDecorate {{.*}} 0 ColMajor
//SPC: OpMemberDecorate {{.*}} 0 RowMajor

//MTR: matrix<float,int(4),int(4)>{{ +}}m_{{[0-9]+}};
//MTC: _MatrixStorage_float4x4_ColMajornatural_{{[0-9]+}} m_{{[0-9]+}};

//WGR: m_{{[0-9]+}} : _MatrixStorage_float4x4std140_{{[0-9]+}},
//WGC: m_{{[0-9]+}} : _MatrixStorage_float4x4_ColMajorstd140_{{[0-9]+}},
