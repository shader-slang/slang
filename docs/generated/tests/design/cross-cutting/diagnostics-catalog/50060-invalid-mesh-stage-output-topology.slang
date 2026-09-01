//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=28ed686b955ce207411fd5446bf9c9d8e616f6584505e15527e663d38ac7c0a1
//META: purpose=Fires diagnostic E50060 (invalid-mesh-stage-output-topology) - invalid mesh output topology
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=50060
//META: catalog_name=invalid-mesh-stage-output-topology
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An [outputtopology] string that is not one of the supported
// values (line/triangle) is rejected at mesh-stage code-gen.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage mesh

struct V { float4 pos : SV_POSITION; };

[outputtopology("bogus")]
[numthreads(1,1,1)]
[shader("mesh")]
void main(out vertices V verts[3], out indices uint3 tris[1]) {
    SetMeshOutputCounts(0,0);
}

//CHECK: E50060
