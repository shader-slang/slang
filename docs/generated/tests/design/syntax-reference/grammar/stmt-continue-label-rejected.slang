//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=c4108d6661545bac260ff71845e7ba2fe35ca7f7587b33e25c895a81925cb89d
//META: purpose=`ContinueStmt` parses as `'continue' ';'` with no optional label, unlike `BreakStmt`; a labeled continue is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the asymmetry the production calls out: BreakStmt admits an
// optional IDENT label but ContinueStmt does not. An identifier after
// `continue` must trigger the parser's unexpected-token diagnostic
// E20001 where a `;` was required.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    for (int i = 0; i < 4; i++)
    {
        continue again;
        //CHECK: error
        //CHECK: E20001
    }
}
