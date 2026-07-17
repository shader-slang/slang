//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=8cbc53779444646ce64a0d14a5a1f170a96bcb2eec03025e1840f339a2581e40
//META: purpose=An ExpressionStmt holds an arbitrary `Expr` used for its side effects; the value of the expression is computed and discarded, and any side effects (function call, assignment) are observed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says ExpressionStmt "lets an arbitrary Expr be used for its
// side effects". A non-void function call whose return is unused is
// the canonical example: the call still runs (printing the side
// effect), and the program continues past the unused result.

//TEST:INTERPRET(filecheck=CHECK):

int sideEffectingCall(int v)
{
    printf("called-with=%d\n", v);
    return v * 100;
}

void main()
{
    // The return value is intentionally unused; the call is the side
    // effect we want to verify.
    sideEffectingCall(7);
    // CHECK: called-with=7
    printf("after\n");
    // CHECK: after
}
