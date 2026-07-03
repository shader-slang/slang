//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=Two MatrixExpressionTypes with different row/column counts are distinct; assigning a float2x2 to a float3x3 is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the row/column operands of MatrixExpressionType:
// a float2x2 and a float3x3 differ in both counts, so an implicit
// assignment across shapes has no conversion and is rejected. CHECK
// pins the type-mismatch diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    float2x2 a = float2x2(1, 2, 3, 4);
    float3x3 b = a;
//CHECK:         ^ type mismatch in expression
//CHECK:         ^ expected an expression of type 'matrix<float,3,3>', got 'matrix<float,2,2>'
}
