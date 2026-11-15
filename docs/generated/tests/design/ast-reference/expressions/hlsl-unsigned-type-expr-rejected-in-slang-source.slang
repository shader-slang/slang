//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:11+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: doc_section_digest=c302dc74b058ca543a5a7a87698d341622b83a61a0da5b402a400dbeccc44d4c
//META: purpose=The `unsigned` type specifier is produced only for HLSL source, so a Slang source file that writes `unsigned int` is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies HLSLUnsignedTypeExpr is HLSL-only: in a `.slang` file (Slang
// language mode) `unsigned` is an ordinary identifier, so `unsigned int a`
// fails to parse and then fails name lookup. CHECK-DAG pins the parse error
// and the undefined-identifier code so an unrelated failure cannot satisfy it.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    unsigned int a = 1u;
    outputBuffer[0] = a;
}

// CHECK-DAG: E20001
// CHECK-DAG: E30015
