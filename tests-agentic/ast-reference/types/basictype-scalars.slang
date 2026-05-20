//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=Each BasicExpressionType (int, uint, float, bool) carries the value of its scalar built-in; the value flows through evaluation as that type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `BasicExpressionType` is the arithmetic-type leaf for the scalar
// built-ins (int, uint, float, bool, void, ...). Each surface
// spelling produces a value typed as the corresponding scalar; we
// group several scalars in one test rather than emit one file per
// scalar.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = -5;
    uint u = 7u;
    float f = 1.5f;
    bool b = true;

    // CHECK: i=-5
    printf("i=%d\n", i);
    // CHECK: u=7
    printf("u=%d\n", int(u));
    // CHECK: f=1
    printf("f=%d\n", int(f));
    // CHECK: b=1
    printf("b=%d\n", b ? 1 : 0);
}
