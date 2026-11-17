//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
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
