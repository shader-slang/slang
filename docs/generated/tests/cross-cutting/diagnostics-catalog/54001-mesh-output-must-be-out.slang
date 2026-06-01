//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E54001 (mesh-output-must-be-out) - mesh output must be out
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=54001
//META: catalog_name=mesh-output-must-be-out
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Mesh-shader vertex/index outputs must use the `out` direction.
// Declaring them with `in` triggers the must-be-out warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage mesh

struct V { float4 pos : SV_POSITION; };

[outputtopology("triangle")]
[numthreads(1,1,1)]
[shader("mesh")]
void main(in vertices V verts[3], in indices uint3 tris[1]) {
    SetMeshOutputCounts(0,0);
}

// CHECK: E54001
