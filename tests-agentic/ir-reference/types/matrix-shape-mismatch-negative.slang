//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Negative: assigning a float2x2 to a float3x3 is rejected because Mat types with different rowCount/columnCount are distinct.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Mat(elementType, rowCount, columnCount, layout)`
// has separate row and column operands. Two `Mat` types with
// different row/column shapes are distinct IR types — assignment
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
