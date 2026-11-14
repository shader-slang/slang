//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`LabelStmt ::= IDENT ':' Stmt` names a loop that the `IDENT` arm of `BreakStmt ::= 'break' IDENT? ';'` then exits.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two halves of the labeled-break story together: an
// identifier followed by `:` in statement position labels the loop
// that follows it, and `break <label>;` targets that outer loop rather
// than the innermost one. The counter distinguishes the two readings:
// an unlabeled break would leave the outer loop running for all three
// iterations and reach 3, while breaking out of `outer` stops the
// whole nest on the first pass.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int passes = 0;
    outer:
    for (int i = 0; i < 3; ++i)
    {
        passes += 1;
        for (int j = 0; j < 3; ++j)
        {
            if (j == 1) break outer;
        }
    }
    // CHECK: passes=1
    printf("passes=%d\n", passes);
}
