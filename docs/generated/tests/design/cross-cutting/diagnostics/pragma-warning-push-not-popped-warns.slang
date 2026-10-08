//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=969ca4caf68f403a893def98e7ed795e3b49ea96059129edfb3f246dc1cdcd15
//META: purpose=Boundary: a `#pragma warning(push)` left unpopped at end of file is reported as E15612.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other boundary of the per-source warning-state stack: a
// `#pragma warning(push)` never popped before end of file is diagnosable.
// The caret-anchored CHECK pins code 15612 and the unmatched-push message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(push)
//CHECK:        ^^^^ E15612
//CHECK:        ^^^^ detected #pragma warning(push) with no corresponding #pragma warning(pop)

void main() {}
