//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizemeshoutputtypes
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=legalizeMeshOutputTypes is gated on reqSet.meshOutput and runs in Phase C for a mesh-stage entry point; the HLSL emission preserves the explicit per-vertex and per-primitive output declarations and the SetMeshOutputCounts call.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Phase C row 12 lists `legalizeMeshOutputTypes` gated on
// `reqSet.meshOutput`. A `[shader("mesh")]` entry point with
// per-vertex/per-primitive outputs and `SetMeshOutputCounts(...)`
// triggers the pass. We pin the canonical HLSL mesh attributes plus
// the `out indices` / `out vertices` declarations and the call to
// `SetMeshOutputCounts(...)` in the emit.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage mesh -entry main

struct MeshVertex
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

struct MeshPrim
{
    uint primId : SV_PrimitiveID;
};

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
    uint tid : SV_DispatchThreadID,
    out indices uint3 triangles[1],
    out vertices MeshVertex verts[3],
    out primitives MeshPrim prims[1])
{
    SetMeshOutputCounts(3, 1);
    if (tid < 3)
    {
        verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
        verts[tid].color = float3(float(tid), 1.0, 0.0);
    }
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].primId = 42;
    }
}

// CHECK-DAG: outputtopology("triangle")
// CHECK-DAG: numthreads(3, 1, 1)
// CHECK-DAG: out indices
// CHECK-DAG: out vertices
// CHECK-DAG: SetMeshOutputCounts
