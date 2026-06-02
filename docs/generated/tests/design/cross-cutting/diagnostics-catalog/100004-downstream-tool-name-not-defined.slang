//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/compiler-core/slang-misc-diagnostic-defs.h
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E100004 (downstreamToolNameNotDefined) - downstream tool name not defined
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=100004
//META: catalog_name=downstreamToolNameNotDefined
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-X...` (open with empty tool name) is rejected because no
// downstream tool is named between the `-X` and the trailing dots.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute -X...

[numthreads(1,1,1)]
void main() {}

// CHECK: 100004
