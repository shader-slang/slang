//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=f952036c11ff9b8c1d73b32cb9eb2592525bdd2e44c5b243e4cfdec0a9352108
//META: purpose=The sink decodes the `SourceLoc` through `SourceManager` and renders a caret at the offending token's column.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the caret is placed at the column of the offending token. The
// undeclared name `b` starts at column 13; the annotation puts a single caret
// there, and the runner compares annotated column against reported column, so
// any drift in the decoded column fails the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = b;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'b'.
}
