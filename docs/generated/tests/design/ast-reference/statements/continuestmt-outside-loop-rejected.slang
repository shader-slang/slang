//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A ContinueStmt is a ChildStmt whose target must be an enclosing LoopStmt; used outside any loop the checker rejects it because no matching enclosing loop exists.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ContinueStmt is documented as a JumpStmt that derives from
// ChildStmt; unlike BreakStmt, `continue` only targets a LoopStmt
// (not a SwitchStmt). Outside any loop there is no matching target,
// so the checker must reject it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    continue;
/*CHECK:
    ^^^^^^^^ 'continue' must appear inside loop constructs
    ^^^^^^^^ 'continue' must appear inside loop constructs.
*/
// CHECK: E30004
}
