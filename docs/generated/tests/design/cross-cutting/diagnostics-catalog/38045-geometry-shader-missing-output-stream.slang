//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=26702b9d07ab7f1914ac321f2eaf5df7f5a2690bc0d2667bae6081608146333d
//META: purpose=Fires diagnostic E38045 (geometry-shader-missing-output-stream) - geometry shader missing output stream parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38045
//META: catalog_name=geometry-shader-missing-output-stream
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A geometry shader entry point with no output stream parameter
// triggers E38045.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage geometry -entry main -profile sm_5_0

[maxvertexcount(1)]
void main(triangle float4 verts[3] : SV_Position) {}

//CHECK: E38045
