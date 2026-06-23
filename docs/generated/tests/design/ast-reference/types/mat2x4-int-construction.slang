//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<int, 2, 4>: rectangular shape with integer element type; each cell addressable.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular shape (R=2, C=4) with int element type
// confirms the (element-type, rows, cols) encoding accepts
// non-float element types in rectangular shapes.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    matrix<int, 2, 4> m = matrix<int, 2, 4>(10, 20, 30, 40, 50, 60, 70, 80);
    // CHECK: m=10,20,30,40|50,60,70,80
    printf("m=%d,%d,%d,%d|%d,%d,%d,%d\n",
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3]);
}
