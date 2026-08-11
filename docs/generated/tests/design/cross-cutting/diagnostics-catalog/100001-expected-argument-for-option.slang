//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-misc-diagnostic-defs.h
//META: doc_section_digest=eec110e3673651ee02a2bed7f192a450bf916be96ae1fac673cc3002f2a2352e
//META: purpose=Fires diagnostic E100001 (expectedArgumentForOption) - expected an argument for command-line option
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=100001
//META: catalog_name=expectedArgumentForOption
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-profile` requires a following value. Omitting it triggers the
// expected-argument diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -profile

[numthreads(1,1,1)]
void main() {}

//CHECK: 100001
