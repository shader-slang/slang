//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
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
