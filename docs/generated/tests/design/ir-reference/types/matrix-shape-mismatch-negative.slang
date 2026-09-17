//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=d0336b3ea55b3ff0fce576141345cc9ee2235732d9657d0e6dc4ff8430e31a5e
//META: purpose=Negative: assigning a float2x2 to a float3x3 is rejected because Mat types with different rowCount/columnCount are distinct.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// assigning a float2x2 to a float3x3 is rejected because Mat types
// with different rowCount/columnCount are distinct. Two `Mat` types
// with different row/column shapes are distinct IR types — assignment
// without explicit conversion fires a type-mismatch diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float2x2 m22;
    float3x3 m33 = m22;
//CHECK:           ^^^ type mismatch in expression
//CHECK:           ^^^ expected an expression of type 'matrix<float,3,3>', got 'matrix<float,2,2>'
// CHECK: E30019
    buf[0] = m33[0][0];
}
