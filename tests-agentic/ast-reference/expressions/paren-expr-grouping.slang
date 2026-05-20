//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A ParenExpr `(e)` groups a sub-expression and overrides the default operator precedence at evaluation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists ParenExpr as `(e)` "preserved explicitly to keep
// rewriter output stable". Its semantic role is also to override
// precedence: `(2 + 3) * 4` differs from `2 + 3 * 4`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int withParens = (2 + 3) * 4;
    int noParens   = 2 + 3 * 4;
    // CHECK: paren=20
    printf("paren=%d\n", withParens);
    // CHECK: noparen=14
    printf("noparen=%d\n", noParens);
}
