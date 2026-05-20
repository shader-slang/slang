//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Boundary: an unknown `#pragma warning` specifier renders E15613 with the actual offending specifier name interpolated.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the edge of the pragma-warning suppression surface: when
// the specifier isn't one of `push`, `pop`, `disable`, `error`,
// `default`, `suppress`, etc., the preprocessor emits 15613 with
// the offending specifier interpolated via `~specifier`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(maybe: 41024)
//CHECK:        ^^^^^ 15613
//CHECK:        ^^^^^ unknown #pragma warning specifier 'maybe'

void main() {}
