//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=d8418d47e63070ba0cf68467df4c96fca1462a9b09b58fcda9b0f2dcaac6484b
//META: purpose=Boundary: an unrecognised `#pragma warning` specifier is reported as E15613 with the offending specifier interpolated.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the edge of the pragma-warning suppression surface: when
// the specifier isn't one of `push`, `pop`, `disable`, `error`,
// `default`, `suppress`, etc., the preprocessor emits 15613 with
// the offending specifier interpolated via `~specifier`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(maybe: 41024)
//CHECK:        ^^^^^ E15613
//CHECK:        ^^^^^ unknown #pragma warning specifier 'maybe'

void main() {}
