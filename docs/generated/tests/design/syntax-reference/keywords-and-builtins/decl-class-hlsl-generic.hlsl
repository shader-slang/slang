//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=In HLSL source `class` is parsed by `ParseStruct` with the full struct grammar, so a generic `class Box<T>` is accepted and its field value reaches the emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// In HLSL source language the parser routes `class` through
// `ParseStruct`, which accepts a generic parameter list. The same
// declaration is rejected in Slang source (see
// `negative-class-generic-slang-source.slang`).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

//CHECK: outBuf_0[int(0)] = 3.0f
class Box<T>
{
    T v;
};

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    Box<float> b;
    b.v = 3.0;
    outBuf[0] = b.v;
}
