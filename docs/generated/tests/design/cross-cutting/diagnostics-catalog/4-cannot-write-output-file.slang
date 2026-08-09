//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6762ad4da8c22c2cd8cca47a441ae441a995003283c6c3708593c990da346792
//META: purpose=Fires diagnostic E00004 (cannot-write-output-file) - cannot write output file '~path'
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=4
//META: catalog_name=cannot-write-output-file
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The driver reports a distinct code when the requested output path cannot be
// opened for writing. Compiling into a directory that does not exist makes the
// write fail after codegen succeeds, so the CHECK pins the write-side code
// rather than any front-end error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -o /nonexistent-dir-xyz/out.hlsl
[numthreads(1,1,1)]
void main() {}

//CHECK: E00004
