//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=Outside HLSL source `class` goes through `ParseClass`, which consumes no generic parameter list, so `class Box<T>` is a parse error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Counterpart of `decl-class-hlsl-generic.hlsl`: in Slang source
// `ParseClass` reads a name and then expects `{`, so `<` is unexpected.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):

//CHECK: E20001
class Box<T>
{
    T v;
};

void main() {}
