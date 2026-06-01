//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30703 (per-primitive-semantic-in-vertex-output) -- per-primitive system value semantic '~semantic' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter, not in a vertex or index output
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30703
//META: catalog_name=per-primitive-semantic-in-vertex-output
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct V { float4 p : SV_Position; uint i : SV_PrimitiveID; }
[shader("mesh")][outputtopology("triangle")][numthreads(1,1,1)]
void main(out vertices V verts[3], out indices uint3 idx[1]) {}


// CHECK: E30703
