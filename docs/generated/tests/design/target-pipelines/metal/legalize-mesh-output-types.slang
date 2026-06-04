//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=legalizeMeshOutputTypes runs for Metal mesh entry points (reqSet.meshOutput): the HLSL-style `out vertices`/`out indices` outputs lower to Metal's `metal::mesh<...>` argument type and the entry point carries the `[[mesh]]` attribute.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A Metal mesh entry point exercises the Phase-C
// `legalizeMeshOutputTypes` pass. The lowering produces a
// `metal::mesh<Verts, Prims, MaxVerts, MaxPrims, topology>`
// argument and the `[[mesh]]` attribute on the kernel.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage mesh

struct Vertex
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void main(
    in uint tig : SV_GroupIndex,
    out vertices Vertex verts[3],
    out indices uint3 tris[1])
{
    SetMeshOutputCounts(3u, 1u);
    if (tig < 3u)
    {
        Vertex v;
        v.pos = float4(float(tig), 0.0, 0.0, 1.0);
        v.color = float3(1.0, 0.0, 0.0);
        verts[tig] = v;
    }
    if (tig < 1u)
    {
        tris[tig] = uint3(0u, 1u, 2u);
    }
}

// CHECK: #include <metal_stdlib>
// CHECK: mesh
// CHECK: void main_0
