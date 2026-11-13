//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=A diagnostic declared at `Severity::Error` renders with the user-facing severity name `error`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the severity name rendered for an entry declared with the `err`
// helper. Undefined-identifier (E30015) is such an entry; the caret-anchored
// CHECK asserts the severity word reported at the offending column is `error`,
// which is the user-facing name of `Severity::Error`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = b;
//CHECK:    ^ error
//CHECK:    ^ undefined identifier
}
