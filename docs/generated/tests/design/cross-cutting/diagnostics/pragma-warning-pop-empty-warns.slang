//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=d8418d47e63070ba0cf68467df4c96fca1462a9b09b58fcda9b0f2dcaac6484b
//META: purpose=Boundary: a `#pragma warning(pop)` with an empty warning-state stack is itself reported as E15611.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the warning-state stack the sink tracks per source:
// a `#pragma warning(pop)` with nothing pushed is itself diagnosable. The
// caret-anchored CHECK pins code 15611 and the unmatched-pop message at the
// pragma's column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(pop)
//CHECK:        ^^^ E15611
//CHECK:        ^^^ detected #pragma warning(pop) with no corresponding #pragma warning(push)

void main() {}
