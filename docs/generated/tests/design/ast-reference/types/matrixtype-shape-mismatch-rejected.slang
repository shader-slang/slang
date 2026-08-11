//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=Two MatrixExpressionTypes with different row/column counts are distinct; assigning a float2x2 to a float3x3 is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the row and column operands: float2x2 and float3x3
// differ in both counts and there is no implicit conversion between
// them. CHECK pins the type-mismatch pair, whose rendered types spell
// out both shapes.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    float2x2 a = float2x2(1, 2, 3, 4);
    float3x3 b = a;
//CHECK:         ^ type mismatch in expression
//CHECK:         ^ expected an expression of type 'matrix<float,3,3>', got 'matrix<float,2,2>'
}
