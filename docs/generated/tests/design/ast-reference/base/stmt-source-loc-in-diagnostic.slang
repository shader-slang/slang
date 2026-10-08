//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#stmt-modifiablesyntaxnode
//META: doc_section_digest=8ea5e05b7dcdf6004362460ead8f7e0c770d0d1fd47a885f5710ce2e461e0a0f
//META: purpose=A Stmt inherits a source location, so a statement-level error is reported at the offending statement's own column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other half of what a statement inherits from its base:
// a location. A `break` outside any loop or switch is rejected, and
// the caret span covers the `break` statement where it was written
// rather than the enclosing function. CHECK pins E30003 and the span.

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
