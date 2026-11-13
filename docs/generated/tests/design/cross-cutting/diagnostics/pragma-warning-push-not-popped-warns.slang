//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=d8418d47e63070ba0cf68467df4c96fca1462a9b09b58fcda9b0f2dcaac6484b
//META: purpose=Boundary: a `#pragma warning(push)` left unpopped at end of file is reported as E15612.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other boundary of the per-source warning-state stack: a
// `#pragma warning(push)` never popped before end of file is diagnosable.
// The caret-anchored CHECK pins code 15612 and the unmatched-push message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):
#pragma warning(push)
//CHECK:        ^^^^ E15612
//CHECK:        ^^^^ detected #pragma warning(push) with no corresponding #pragma warning(pop)

void main() {}
