//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=75a7fb5b09844d509ac3164e91acc685f05e3a7ca3c5ec40f4a710363a1fe013
//META: purpose=Fires diagnostic E30703 (per-primitive-semantic-in-vertex-output) -- per-primitive system value semantic '~semantic' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter, not in a vertex or index output
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30703
//META: catalog_name=per-primitive-semantic-in-vertex-output
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct V { float4 p : SV_Position; uint i : SV_PrimitiveID; }
[shader("mesh")][outputtopology("triangle")][numthreads(1,1,1)]
void main(out vertices V verts[3], out indices uint3 idx[1]) {}


//CHECK: E30703
