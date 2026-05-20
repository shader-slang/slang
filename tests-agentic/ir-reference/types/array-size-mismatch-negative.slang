//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=Negative: assigning an int[3] to an int[4] is rejected because Array types with different elementCount are distinct.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Array(elementType, elementCount)` is parameterised
// by elementCount. Two `Array` types with different
// `elementCount` are distinct IR types, so a cross-extent
// assignment fires a type-mismatch diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int a[3] = { 1, 2, 3 };
    int b[4] = a;
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'int[4]', got 'int[3]'
    buf[0] = b[0];
}
