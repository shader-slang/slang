//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=eb9e797ca9c3dbb949a62072e232852c0f820f49dd331388c5e7ffcde24a8c1c
//META: purpose=Two VectorExpressionTypes with different counts are distinct types; assigning a float2 to a float3 without a conversion is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the count operand: float2 and float3 differ only in
// their element count, and no implicit widening exists between them, so
// the assignment must be rejected. CHECK pins the type-mismatch pair,
// whose rendered types name both counts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    float2 a = float2(1.0, 2.0);
    float3 b = a;
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'vector<float,3>', got 'vector<float,2>'
}
