//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Every diagnostic id lives in one shared integer namespace rendered as `E<code>`; undefined-identifier renders E30015.
//META: intent=functional
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
