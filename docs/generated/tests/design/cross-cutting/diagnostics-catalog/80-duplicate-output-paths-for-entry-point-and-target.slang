//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=11146c02edef75bf885c42eac8eda7da33aa64dbd737eb22970316a202ddfd52
//META: purpose=Fires diagnostic E00080 (duplicate-output-paths-for-entry-point-and-target) - multiple output paths have been specified entry point '~entryPoint:Name' on target '~target'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=80
//META: catalog_name=duplicate-output-paths-for-entry-point-and-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One entry point on one target has a single kernel output. Supplying two `-o`
// paths after the same `-entry` is ambiguous, and the CHECK pins the
// duplicate-output-path error that rejects it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -o a.hlsl -o b.hlsl
[numthreads(1,1,1)]
void main() {}

//CHECK: E00080
