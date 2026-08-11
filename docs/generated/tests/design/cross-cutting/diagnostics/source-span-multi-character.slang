//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=The rendered caret run spans the full offending token rather than only its first column.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the rendered caret run covers the whole offending token rather
// than only its first column. The undeclared name is 18 characters long and
// the annotation supplies exactly 18 carets; the runner compares the annotated
// column range against the reported one, so a one-column span would fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = longUndeclaredName;
//CHECK:    ^^^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^^^ undefined identifier 'longUndeclaredName'.
}
