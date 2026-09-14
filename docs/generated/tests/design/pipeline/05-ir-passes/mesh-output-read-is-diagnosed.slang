//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=667952fd5b9698b654c70c639e18194281145ed55e05baa506681d6bf7a7125d
//META: purpose=The mesh-output-reads validation pass diagnoses a read from a mesh-shader output array.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the mesh-output-reads row of the linking-and-validation
// table. The entry point writes a vertex into its `OutputVertices` array
// and then reads the same element back; the CHECK pins the reported
// message text at the caret the compiler reports, proving the validation
// fires rather than the read silently lowering. The compiler emits a
// short and a long form of the same message at that span, so the
// directive is `non-exhaustive` and only the short form is annotated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage mesh -entry meshMain

uniform RWStructuredBuffer<float> outBuf;

struct Vertex
{
    float4 pos : SV_Position;
    float3 color : Color;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void meshMain(
    in uint tig : SV_GroupIndex,
    OutputVertices<Vertex, 3> verts,
    OutputIndices<uint3, 1> tris)
{
    SetMeshOutputCounts(3, 1);
    verts[tig] = { float4(float(tig), 0, 0, 1), float3(1, 0, 0) };
    outBuf[0] = verts[tig].pos.x;
//CHECK:                       ^ Cannot read values from mesh shader outputs
    if (tig == 0)
        tris[0] = uint3(0, 1, 2);
}
