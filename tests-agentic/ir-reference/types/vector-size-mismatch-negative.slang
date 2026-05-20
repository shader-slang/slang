//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=A vector<int,3>-to-vector<int,4> assignment is rejected; Vec types with different elementCount are not interchangeable.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "`Vec` and `Mat`" note says `Vec(elementType,
// elementCount)` is parameterised by element count. Two `Vec`
// values with different `elementCount` are different IR types and
// the type system rejects the assignment of one to the other.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int3 v = int3(1, 2, 3);
    int4 w = v;
//CHECK:     ^ type mismatch in expression
//CHECK:     ^ expected an expression of type 'vector<int,4>', got 'vector<int,3>'
    buf[0] = w.x;
}
