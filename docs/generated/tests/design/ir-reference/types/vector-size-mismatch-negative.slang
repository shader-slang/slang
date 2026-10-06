//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=d0336b3ea55b3ff0fce576141345cc9ee2235732d9657d0e6dc4ff8430e31a5e
//META: purpose=A vector<int,3>-to-vector<int,4> assignment is rejected; Vec types with different elementCount are not interchangeable.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A vector<int,3>-to-vector<int,4> assignment is rejected; Vec types
// with different elementCount are not interchangeable. Two `Vec`
// values with different `elementCount` are different IR types and the
// type system rejects the assignment of one to the other.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int3 v = int3(1, 2, 3);
    int4 w = v;
//CHECK:     ^ type mismatch in expression
//CHECK:     ^ expected an expression of type 'vector<int,4>', got 'vector<int,3>'
// CHECK: E30019
    buf[0] = w.x;
}
