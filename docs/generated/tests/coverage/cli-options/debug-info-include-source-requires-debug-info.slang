//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=d592afa9b9efaf43740554b05a3fdf5b6f6e5077
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Verifies -debug-info-include-source combined with -g0 is rejected with E57007, the cross-option validation that runs after both options parse successfully.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of a cross-option consistency check, as opposed to the
// per-option value parse pinned by the sibling -O9 / -profile tests. Both
// `-g0` and `-debug-info-include-source` parse cleanly on their own; the
// rejection happens afterwards, when the option set as a whole is validated
// and the request to embed source is found to have no debug information to
// embed it into. The diagnostic therefore carries no source location -- it is
// reported against the command line, so the CHECK pins the code alone.
//
// `-debug-info-include-source` was added in #12202 to decouple source
// embedding from the `-g` level; this test pins the guard that came with it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -g0 -debug-info-include-source

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

//CHECK: E57007
