//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<int, 4, 4>: square shape with int element type; 16 lanes addressable.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: square 4x4 shape with int element type. Probes
// the (int, 4, 4) operand triple. Sums every cell to confirm
// per-cell addressability.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    matrix<int, 4, 4> m;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            m[r][c] = r*4 + c;
    int s = 0;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            s += m[r][c];
    // CHECK: s=120
    printf("s=%d\n", s);
}
