//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=Two VectorExpressionTypes with different counts are distinct types; assigning a float2 to a float3 without a conversion is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the count operand of VectorExpressionType: a
// float2 and a float3 differ in count, so an implicit assignment from
// the shorter to the longer has no conversion and is rejected. CHECK
// pins the type-mismatch diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    float2 a = float2(1.0, 2.0);
    float3 b = a;
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'vector<float,3>', got 'vector<float,2>'
}
