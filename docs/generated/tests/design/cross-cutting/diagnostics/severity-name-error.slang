//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=a2c5b6b64f38a8b7fd3692290be87fe1f26f4a31a5f12e076913b5c584acc7f3
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
