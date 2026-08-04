//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=f4e4743214fa16b25679c231bf78519febf5f40d591d803e8822ade1f048c8fc
//META: purpose=Verifies an unrecognized slangc command-line option is rejected with E00017 naming the unknown option.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the option parser's fall-through path in slang-options.cpp:
// an option string that matches no known OptionKind diagnoses E00017. The
// diagnostic is attached to the synthetic command line (source line 0), so it is
// pinned by error code; the observed message text is recorded verbatim in the
// comment below as the characterization record (the matcher matches the code).
// Observed message: "unknown command-line option '-no-such-option'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -no-such-option -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

//CHECK: E00017
