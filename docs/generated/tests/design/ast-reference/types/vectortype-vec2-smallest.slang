//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType at the smallest documented vector dimension (N=2) carries two independently-addressable components.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small end of vector dim: a `float2`
// VectorExpressionType has two components addressable as `.x .y`.
// Complements the existing `float4` smoke (vectortype-components).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2 v = float2(7.0, 8.0);
    // CHECK: v=7,8
    printf("v=%d,%d\n", int(v.x), int(v.y));
}
