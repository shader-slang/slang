//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4110bd6574db580902317bafdbb8713ab143ca04d123940d64579351bf5f47e0
//META: purpose=Fires diagnostic E54003 (mesh-output-array-must-have-size) - mesh output array must have size
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=54003
//META: catalog_name=mesh-output-array-must-have-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL-style mesh output arrays must declare a length. An unbounded
// `[]` array triggers the missing-size error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage mesh

struct V { float4 pos : SV_POSITION; };

[outputtopology("triangle")]
[numthreads(1,1,1)]
[shader("mesh")]
void main(out vertices V verts[]) {
    SetMeshOutputCounts(0,0);
}

//CHECK: E54003
