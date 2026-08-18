//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A BreakStmt is a ChildStmt whose target must be an enclosing BreakableStmt (loop or switch); used outside any such enclosing statement it is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `break` requires an enclosing loop or switch to jump out of.
// With no such enclosing statement, checking has nothing to match the
// jump against; the CHECK pins the E30003 diagnostic on the `break`
// token itself.

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
