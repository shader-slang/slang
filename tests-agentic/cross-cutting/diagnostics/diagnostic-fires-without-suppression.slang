//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=9c56cd15864837ba13d0f3ca512f679e11adc378d62d03ad3125087b47fe77e6
//META: purpose=The sink emits diagnostics by default; without an override the comma-operator warning fires as expected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive control for the suppression test: without a
// `#pragma warning(disable: ...)` the same source triggers the
// warning. This locks in the precondition that the suppression
// test was actually changing behavior.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int x = (1, 2);
//CHECK:      ^ 41024
//CHECK:      ^ comma operator used in expression
// CHECK: W41024
    return x;
}
