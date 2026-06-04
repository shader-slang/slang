//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/compiler-core/slang-misc-diagnostic-defs.h
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E100000 (downstreamNameNotKnown) - downstream tool name not known
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=100000
//META: catalog_name=downstreamNameNotKnown
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-X<name>` requires <name> to be a known downstream tool. A made-
// up name is rejected at command-line parsing time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute -Xbogus

[numthreads(1,1,1)]
void main() {}

// CHECK: 100000
