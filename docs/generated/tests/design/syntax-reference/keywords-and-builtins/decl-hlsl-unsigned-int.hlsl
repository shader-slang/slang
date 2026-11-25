//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T08:46:10+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=In HLSL source `unsigned int` and bare `unsigned` are accepted by the type-specifier parser and name the unsigned integer type.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The type-specifier parser's first branch, when the source language is
// HLSL, recognizes `unsigned` through
// `isHLSLTraditionalIntegerTypeSpecifierStart`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

//CHECK: RWStructuredBuffer<uint >
//CHECK: outBuf_0[int(0)] = tid_0.x + 6U

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    unsigned int a = tid.x;
    unsigned b = a + 6;
    outBuf[0] = b;
}
