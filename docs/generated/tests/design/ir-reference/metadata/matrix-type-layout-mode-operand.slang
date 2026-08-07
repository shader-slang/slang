//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=8133b77a147a20aa12fcc20641167e9fc2f42707469a60ef2b36207749f54a95
//META: purpose=A matrixTypeLayout stores a MatrixLayoutMode integer literal in operand 0, which changes with the requested default matrix layout.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies operand 0 of a matrix layout really is the layout mode and not an
// attribute. The same shader is compiled twice with the two documented
// defaults; the mode literal differs between them (row-major and column-major
// select different enumerators) while the trailing operands stay attributes.
// A test that only pinned one mode could not tell a mode operand from a
// constant.

//TEST:SIMPLE(filecheck=ROW):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=COL):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -matrix-layout-column-major

cbuffer CB
{
    float4x4 m;
}

uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = mul(m, float4(0, 0, 0, 1));
}

// ROW: = matrixTypeLayout(1 : Int, %[[RSZ:[0-9]+]], %{{[0-9]+}})
// ROW: %[[RSZ]]{{.*}}= size(8 : Int, 64 : Int)

// COL: = matrixTypeLayout(2 : Int, %[[CSZ:[0-9]+]], %{{[0-9]+}})
// COL: %[[CSZ]]{{.*}}= size(8 : Int, 64 : Int)
