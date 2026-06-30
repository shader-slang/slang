//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Negative: assigning a vector<int,4> to a vector<int,2> (narrowing) is rejected by the type checker.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Vec(elementType, elementCount)` is parameterised
// by elementCount; two Vec types of different elementCount are
// distinct IR types. A narrowing assignment from
// `vector<int,4>` to `vector<int,2>` (without explicit cast)
// fires a type-mismatch diagnostic — the inverse of the
// widening case covered by `vector-size-mismatch-negative.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int4 v = int4(1, 2, 3, 4);
    int2 w = v;
//CHECK:     ^ type mismatch in expression
//CHECK:     ^ expected an expression of type 'vector<int,2>', got 'vector<int,4>'
// CHECK: E30019
    buf[0] = w.x;
}
