//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=e05101999cc6555f3e7214b2fb06744dbaf16788905b25807e20594acfdd10d6
//META: purpose=An ExpressionStmt holds an arbitrary `Expr` used for its side effects; the value of the expression is computed and discarded, and any side effects (function call, assignment) are observed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an expression may stand alone as a statement purely for its
// side effect. A non-void call's result is discarded; the CHECKs assert
// the call's own print fired and that execution continued past the
// discarded result.

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
