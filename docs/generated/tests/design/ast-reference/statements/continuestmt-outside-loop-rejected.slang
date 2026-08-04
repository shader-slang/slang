//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
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
