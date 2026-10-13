//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Verifies an out-of-range optimization level (-O9) is rejected with E00062 listing the accepted -O values.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the value-category parse for OptionKind::Optimization.
// -O accepts a fixed enumerated value set; an unrecognized level falls through
// to E00062 ("unknown value for option"). The diagnostic is on the synthetic
// command line (line 0) and is pinned by error code. Observed message recorded
// verbatim: "unknown value for option '-O'. Valid values are
// '0, none, 1, default, 2, high, 3, maximal'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -O9 -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

//CHECK: E00062
