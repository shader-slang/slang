//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=A diagnostic declared at `Severity::Warning` renders with the user-facing severity name `warning`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the severity name rendered for an entry declared with the `warning`
// helper. A comma operator in a value context triggers E41024; the
// caret-anchored CHECK asserts the severity word reported at the comma's column
// is `warning`, the user-facing name of `Severity::Warning`, distinguishing it
// from the `error` rendering asserted in severity-name-error.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):
int main()
{
    int x = (1, 2);
//CHECK:      ^ warning
//CHECK:      ^ comma operator used in expression
    return x;
}
