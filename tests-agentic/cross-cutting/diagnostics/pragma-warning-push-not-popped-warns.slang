//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Boundary: a `#pragma warning(push)` with no matching pop emits E15612 with the "unmatched push" message.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says diagnostic ids are unique per diagnostic. Pin the
// rendered code 15612 for the unmatched-push warning when the
// pragma push has no companion pop in the translation unit.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(push)
//CHECK:        ^^^^ 15612
//CHECK:        ^^^^ detected #pragma warning(push) with no corresponding #pragma warning(pop)
// CHECK: E15612

void main() {}
