//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38045
