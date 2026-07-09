//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:00Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=3c2365d6e03d7d49bdb8070671c9161746cf63482dd5364eff59042e14f93322
//META: purpose=Diagnostic ids live in a single shared integer namespace rendered as E-codes; for undefined-identifier the rendered code is E30015.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a diagnostic's integer id from the shared code namespace is
// rendered as an E-code on the diagnostic line. Pin the rendered code for
// the undefined-identifier diagnostic to E30015 by FileChecking that code
// on the caret line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = b;
//CHECK:    ^ E30015
//CHECK:    ^ undefined identifier
}
