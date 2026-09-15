//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Verifies an unrecognized -line-directive-mode value is rejected with E00062 listing the accepted modes.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the value-category parse for OptionKind::LineDirectiveMode.
// An unknown mode string falls through to E00062. The diagnostic is on the
// synthetic command line (line 0) and is pinned by error code. Observed message
// recorded verbatim: "unknown value for option '-line-directive-mode'. Valid
// values are 'none, source-map, default, standard, glsl'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -line-directive-mode bogus -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

//CHECK: E00062
