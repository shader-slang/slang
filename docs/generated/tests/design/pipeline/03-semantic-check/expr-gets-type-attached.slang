//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#inputs-and-outputs
//META: doc_section_digest=325192a5d2be9d9ec0f3812a36230d5c900a48f1c7af5d97ada17cd1926349b1
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
