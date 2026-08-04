//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38046 (geometry-shader-missing-max-vertex-count) - geometry shader missing [maxvertexcount] attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38046
//META: catalog_name=geometry-shader-missing-max-vertex-count
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A geometry shader entry point without `[maxvertexcount(...)]`
// triggers E38046.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage geometry -entry main -profile sm_5_0

struct V { float4 pos : SV_Position; };

void main(triangle float4 verts[3] : SV_Position, inout TriangleStream<V> outStream) {}

// CHECK: E38046
