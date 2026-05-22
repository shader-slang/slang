//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=DiagnosticSink: the compiler emits diagnostics through a sink; a use of an undeclared identifier produces an "undefined identifier" error, observable as text on the compile output.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "DiagnosticSink" glossary entry: using an
// undeclared identifier `undeclaredThing` causes the checker to
// route a diagnostic through the sink. We assert the diagnostic
// text the user sees, not the sink's internal API.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = undeclaredThing;
    //CHECK: undefined identifier
// CHECK: E30015
}
