//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e0005ff2b050178a20cf58f1f3c108b580aa12dcd65ebbb74f6f41c836839b77
//META: purpose=Fires diagnostic E54002 (mesh-output-must-be-array) - mesh output must be array
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=54002
//META: catalog_name=mesh-output-must-be-array
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL-style mesh outputs must be arrays. A scalar output parameter
// triggers the must-be-array error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage mesh

struct V { float4 pos : SV_POSITION; };

[outputtopology("triangle")]
[numthreads(1,1,1)]
[shader("mesh")]
void main(out vertices V vert) {
    SetMeshOutputCounts(0,0);
}

//CHECK: E54002
