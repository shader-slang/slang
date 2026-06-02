//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for float carries the documented IEEE-754 extremes; +0.0/-0.0 compare equal, +/-inf are reachable by 1/0, NaN compares unequal to itself.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the float BasicExpressionType: exercises positive
// zero, negative zero, +inf (via 1/0), -inf, and NaN (via inf-inf).
// All five extreme values are reachable through float-typed
// expressions and produce the documented IEEE-754 comparison results.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float pz = 0.0f;
    float nz = -0.0f;
    float inf = 1.0f / 0.0f;
    float ninf = -1.0f / 0.0f;
    float nan = inf - inf;
    // CHECK: pz_eq_nz=1
    printf("pz_eq_nz=%d\n", (pz == nz) ? 1 : 0);
    // CHECK: inf_gt=1
    printf("inf_gt=%d\n", (inf > 1.0e30f) ? 1 : 0);
    // CHECK: ninf_lt=1
    printf("ninf_lt=%d\n", (ninf < -1.0e30f) ? 1 : 0);
    // CHECK: nan_ne_self=1
    printf("nan_ne_self=%d\n", (nan != nan) ? 1 : 0);
}
