//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Boundary: an unmatched `#pragma warning(pop)` produces the diagnostic E15611 with the documented "unmatched pop" text.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says every diagnostic has a unique integer id. Probe the
// boundary of the pragma-warning suppression surface itself: a bare
// `#pragma warning(pop)` with no preceding `push` is its own
// diagnosable condition (15611).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(pop)
//CHECK:        ^^^ 15611
//CHECK:        ^^^ detected #pragma warning(pop) with no corresponding #pragma warning(push)
// CHECK: E15611

void main() {}
