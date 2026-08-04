//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=f4e4743214fa16b25679c231bf78519febf5f40d591d803e8822ade1f048c8fc
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
