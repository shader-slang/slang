//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A GenericAppExpr `g<T>` applies explicit generic arguments to a generic decl; the resulting specialization is invokable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// GenericAppExpr is documented with `functionExpr: Expr*` and an
// arguments list — the `<...>` brackets after a generic name. The
// explicit specialization `pick<float>(1.0f)` must invoke the float
// specialization and return 2; the int specialization returns 1.

//TEST:INTERPRET(filecheck=CHECK):

int pick<T>(T x)        { return 1; }
int pick<T>(vector<T,2> v) { return 2; }

void main()
{
    float2 v = float2(1.0f, 2.0f);
    // CHECK: v=2
    printf("v=%d\n", pick<float>(v));
    // CHECK: s=1
    printf("s=%d\n", pick<float>(1.0f));
}
