//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
//META: purpose=`ContinueStmt` parses as `'continue' ';'` with no optional label, unlike `BreakStmt`; a labeled continue is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the asymmetry the production calls out: BreakStmt admits an
// optional IDENT label but ContinueStmt does not. An identifier after
// `continue` must trigger the parser's unexpected-token diagnostic
// E20001 where a `;` was required.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 4; i++)
    {
        continue again;
        //CHECK: error
        //CHECK: E20001
    }
}
