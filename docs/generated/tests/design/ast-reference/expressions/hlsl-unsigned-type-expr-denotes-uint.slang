//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:11+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: doc_section_digest=c302dc74b058ca543a5a7a87698d341622b83a61a0da5b402a400dbeccc44d4c
//META: purpose=In HLSL source, `unsigned` and `unsigned int` are type expressions that always denote the built-in `uint`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `unsigned` / `unsigned int` (HLSLUnsignedTypeExpr) resolve to the
// 32-bit unsigned type. The buffer element and a function parameter are
// spelled with each form; every back-end must emit its 32-bit unsigned type,
// which is only possible if the checker typed the expression as `uint`.

//TEST:SIMPLE_EX(filecheck=HLSL): -lang hlsl docs/generated/tests/design/ast-reference/expressions/hlsl-unsigned-type-expr-denotes-uint.slang -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE_EX(filecheck=GLSL): -lang hlsl docs/generated/tests/design/ast-reference/expressions/hlsl-unsigned-type-expr-denotes-uint.slang -target glsl -entry computeMain -stage compute
//TEST:SIMPLE_EX(filecheck=SPV): -lang hlsl docs/generated/tests/design/ast-reference/expressions/hlsl-unsigned-type-expr-denotes-uint.slang -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE_EX(filecheck=METAL): -lang hlsl docs/generated/tests/design/ast-reference/expressions/hlsl-unsigned-type-expr-denotes-uint.slang -target metal -entry computeMain -stage compute
//TEST:SIMPLE_EX(filecheck=WGSL): -lang hlsl docs/generated/tests/design/ast-reference/expressions/hlsl-unsigned-type-expr-denotes-uint.slang -target wgsl -entry computeMain -stage compute

RWStructuredBuffer<unsigned int> outputBuffer;

unsigned addOne(unsigned x)
{
    return x + 1;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    unsigned int a = 4000000000u + tid.x;
    outputBuffer[0] = addOne(a);
}

// HLSL: RWStructuredBuffer<uint >
// HLSL: uint addOne_{{[0-9]+}}(uint x_{{[0-9]+}})
// GLSL: uint addOne_{{[0-9]+}}(uint x_{{[0-9]+}})
// SPV: OpTypeInt 32 0
// SPV: OpIAdd %uint
// METAL: uint addOne_{{[0-9]+}}(uint x_{{[0-9]+}})
// WGSL: fn addOne_{{[0-9]+}}({{ *}}x_{{[0-9]+}} : u32) -> u32
