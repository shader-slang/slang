//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A MatrixExpressionType is parameterised by element-type, row count and column count; a float2x3 value's six elements are reachable by [row][col] indexing.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies MatrixExpressionType carries (element-type, rows, cols):
// a float2x3 has 2*3 elements addressable via [row][col]. All six
// slots are printed; CHECK asserts each reaches the right element,
// proving the row/column shape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2x3 m = float2x3(1, 2, 3, 4, 5, 6);
    // CHECK: m=1,2,3,4,5,6
    printf("m=%d,%d,%d,%d,%d,%d\n",
        int(m[0][0]), int(m[0][1]), int(m[0][2]),
        int(m[1][0]), int(m[1][1]), int(m[1][2]));
}
