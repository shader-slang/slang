//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=a2c5b6b64f38a8b7fd3692290be87fe1f26f4a31a5f12e076913b5c584acc7f3
//META: purpose=A diagnostic declared at `Severity::Warning` renders with the user-facing severity name `warning`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the severity name rendered for an entry declared with the `warning`
// helper. A comma operator in a value context triggers E41024; the
// caret-anchored CHECK asserts the severity word reported at the comma's column
// is `warning`, the user-facing name of `Severity::Warning`, distinguishing it
// from the `error` rendering asserted in severity-name-error.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int x = (1, 2);
//CHECK:      ^ warning
//CHECK:      ^ comma operator used in expression
    return x;
}
