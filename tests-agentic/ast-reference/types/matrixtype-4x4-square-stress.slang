//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType at the standard upper-end square dim (4x4) holds 16 elements; an element written at the far corner [3][3] is independently readable.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe at the large end of the standard matrix dim range:
// a `float4x4` carries 16 elements. We populate every cell via a
// loop and observe the bookend cells [0][0] and [3][3].

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4x4 m;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            m[r][c] = float(r*4 + c);
    // CHECK: tl=0
    printf("tl=%d\n", int(m[0][0]));
    // CHECK: br=15
    printf("br=%d\n", int(m[3][3]));
}
