//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T08:46:10+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=In HLSL source `signed` is diagnosed (E20021) rather than accepted as an integer type specifier.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Counterpart of `decl-hlsl-unsigned-int.hlsl`: the same HLSL-only branch
// accepts `unsigned` but diagnoses `signed`.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    //CHECK: E20021
    signed int a = 5;
    outBuf[0] = a;
}
