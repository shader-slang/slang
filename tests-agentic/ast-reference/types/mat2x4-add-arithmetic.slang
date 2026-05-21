//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<float, 2, 4>: elementwise add of two 2x4 matrices yields elementwise sums per cell.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular 2x4 with elementwise arithmetic.
// Confirms that the operand-encoded (rows=2, cols=4) shape
// supports `+` returning a matching-shape matrix.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2x4 a = float2x4(1, 2, 3, 4, 5, 6, 7, 8);
    float2x4 b = float2x4(10, 20, 30, 40, 50, 60, 70, 80);
    float2x4 c = a + b;
    // CHECK: c=11,22,33,44|55,66,77,88
    printf("c=%d,%d,%d,%d|%d,%d,%d,%d\n",
        int(c[0][0]), int(c[0][1]), int(c[0][2]), int(c[0][3]),
        int(c[1][0]), int(c[1][1]), int(c[1][2]), int(c[1][3]));
}
