//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=A BreakStmt is a ChildStmt whose target must be an enclosing BreakableStmt (loop or switch); used outside any such enclosing statement it is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// BreakStmt is documented as a JumpStmt that derives from ChildStmt;
// the family-hierarchy section says BreakableStmt holds the uniqueID
// that ChildStmt references via `targetOuterStmtID`. Outside any loop
// or switch there is no matching BreakableStmt, so the checker must
// reject the statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    break;
/*CHECK:
    ^^^^^ 'break' must appear inside loop or switch constructs
    ^^^^^ 'break' must appear inside loop or switch constructs.
*/
// CHECK: E30003
}
