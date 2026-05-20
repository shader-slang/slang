//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
