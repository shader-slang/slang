//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A MatrixExpressionType is parameterised by element-type, row count, and column count; a floatRxC value's R*C elements are independently readable via [row][col] indexing.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `MatrixExpressionType` carries `(element-type, rows, cols, layout)`
// in its operand list. The user-facing consequence: a `float2x3`
// value has 2*3=6 elements addressable via the [row][col] subscript.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2x3 m = float2x3(1, 2, 3, 4, 5, 6);
    // CHECK: m=1,2,3,4,5,6
    printf("m=%d,%d,%d,%d,%d,%d\n",
        int(m[0][0]), int(m[0][1]), int(m[0][2]),
        int(m[1][0]), int(m[1][1]), int(m[1][2]));
}
