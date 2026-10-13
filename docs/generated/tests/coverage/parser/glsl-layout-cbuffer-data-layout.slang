//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies that under -allow-glsl a GLSL `layout(...)` data-layout qualifier in front of an HLSL `cbuffer` selects the constant buffer's data layout, for each of std140, std430 and scalar.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `parseHLSLCBufferDecl` normally builds a plain
// `ConstantBuffer`. When GLSL input is allowed *and* the `cbuffer` keyword was
// preceded by modifiers, it instead looks through those pending modifiers for
// a GLSL buffer-data-layout qualifier and passes the matching layout type down
// to `ParseBufferBlockDecl`. This is the one place a GLSL `layout()` qualifier
// and an HLSL `cbuffer` meet, and each of the three recognised qualifiers maps
// to a different layout type.
//
// The chosen layout survives all the way to SPIR-V as a suffix on the
// generated parameter-group type name, so the three `OpName` lines below are a
// direct read-out of which branch the parser took. Note that `scalar` maps to
// `ScalarDataLayout`, which emission names `_natural` rather than `_scalar`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -allow-glsl -entry main -stage compute

layout(std140) cbuffer CB140 { float4 a; }
layout(std430) cbuffer CB430 { float4 b; }
layout(scalar) cbuffer CBscalar { float4 c; }

//CHECK: OpName %SLANG_ParameterGroup_CB140_std140
//CHECK: OpName %SLANG_ParameterGroup_CB430_std430
//CHECK: OpName %SLANG_ParameterGroup_CBscalar_natural

[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<float4> o)
{
    o[0] = a + b + c;
}
