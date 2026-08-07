//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=a39cfe25465518ad38d31423ca65973ffe6e67b11946248f9af71a03f7cee6df
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
// CHECK: E30019
    buf[0] = b[0];
}
