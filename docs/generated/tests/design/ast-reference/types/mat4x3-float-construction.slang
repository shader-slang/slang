//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<float, 4, 3>: 4 rows by 3 columns, 12 elements addressable via [r][c].
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular matrix shape (R=4, C=3). Transpose
// shape of float3x4; both are valid MatrixExpressionType
// instantiations distinct from each other.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4x3 m = float4x3(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    // CHECK: tl=1 br=12
    printf("tl=%d br=%d\n", int(m[0][0]), int(m[3][2]));
}
