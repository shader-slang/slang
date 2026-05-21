//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<float, 3, 4>: 3 rows by 4 columns; far corner [2][3] is independently addressable after a loop fill.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular matrix shape (R=3, C=4). 12 cells are
// independently writable; the bookend cells [0][0] and [2][3]
// survive evaluation.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3x4 m;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 4; ++c)
            m[r][c] = float(r*4 + c);
    // CHECK: tl=0 br=11
    printf("tl=%d br=%d\n", int(m[0][0]), int(m[2][3]));
}
