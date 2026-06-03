//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<float, 3> can hold +inf and NaN per lane; isnan/isinf detect them per component.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: float-edge value probe at the float3 corner. +inf
// and NaN are documented float boundary values that survive in
// VectorExpressionType<float, 3> lanes. We detect inf/nan via
// the documented self-inequality (NaN != NaN) and the larger-
// than-any-finite property of inf, because the library helpers
// isnan/isinf are not selectable under the interpreter profile.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float pinf = 1.0f / 0.0f;
    float nan = 0.0f / 0.0f;
    float3 v = float3(pinf, nan, 1.0f);
    // +inf > any finite
    int isinf_x = (v.x > 1e38f) ? 1 : 0;
    // NaN != itself
    int isnan_y = (v.y != v.y) ? 1 : 0;
    // ordinary finite
    int finite_z = (v.z == 1.0f) ? 1 : 0;
    // CHECK: r=1,1,1
    printf("r=%d,%d,%d\n", isinf_x, isnan_y, finite_z);
}
