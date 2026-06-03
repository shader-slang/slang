//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#inputs-and-outputs
//META: doc_section_digest=85c534d82e8549dc778af3fda4ed6b0174cc56ffa67072e5ed391a2e0a2d7858
//META: purpose=The checker attaches a Type to every Expr; arithmetic between int and float coerces to float and the result type matches that.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the checker has resolved the type of `i + f`: if the
// expression were not type-attached, the implicit int->float coercion
// would not happen and the printed value would be different.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 3;
    float f = 1.5;
    float r = i + f;
    // CHECK: r=4.5
    printf("r=%g\n", r);
}
