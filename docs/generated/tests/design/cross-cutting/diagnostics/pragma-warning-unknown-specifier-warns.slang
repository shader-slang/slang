//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=969ca4caf68f403a893def98e7ed795e3b49ea96059129edfb3f246dc1cdcd15
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
