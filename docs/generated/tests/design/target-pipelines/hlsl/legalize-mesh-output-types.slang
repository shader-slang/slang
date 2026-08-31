//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=53a2379796e273cdb95b30642542c77045c0b9200b2085bfda75a33dd67487c6
//META: purpose=legalizeMeshOutputTypes is gated on reqSet.meshOutput and runs in Phase C for a mesh-stage entry point; the HLSL emission preserves the explicit per-vertex and per-primitive output declarations and the SetMeshOutputCounts call.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `legalizeMeshOutputTypes` fires on the HLSL path when the
// module actually contains mesh-output types (the `reqSet.meshOutput` gate).
// The test declares a mesh entry point with `out vertices` / `out indices`
// parameters and a `SetMeshOutputCounts(...)` call; CHECK asserts the HLSL
// emit keeps the mesh output-topology attribute, both output declarations,
// and the count call, which is the legalized shape DXC accepts.

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
