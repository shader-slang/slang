//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2fc34c8b138e742326827ad62b3427444bb8630a354c575998c6709c96de8c12
//META: purpose=Fires diagnostic E00031 (conflicting-stages-for-entry-point) - conflicting stages have been specified for entry point '~entryPoint'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=31
//META: catalog_name=conflicting-stages-for-entry-point
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two different `-stage` options for the same `-entry` cannot both hold. The
// second `-stage vertex` conflicts with the first `-stage compute`, and the
// CHECK pins the conflicting-stage error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -stage vertex
[numthreads(1,1,1)]
void main() {}

//CHECK: E00031
