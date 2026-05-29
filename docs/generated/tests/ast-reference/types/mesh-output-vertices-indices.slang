//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#geometry--and-tessellation-shader-io-types
//META: doc_section_digest=7c904d8992380b74b7c58a3b82c04f1512a6f31a7f5dec239acb5b75fe6f68d7
//META: purpose=The VerticesType and IndicesType (mesh-shader `vertices`/`indices` outputs) carry the documented per-output element type; the SPIR-V backend lowers a mesh entry point with both outputs to a mesh-shader execution mode.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `VerticesType` and `IndicesType` are the `MeshOutputType` leaves
// for the mesh-shader `out vertices` and `out indices` output
// parameters. Observable via SPIR-V emit at a mesh entry point: the
// MeshEXT execution mode and a writable position output are present.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage mesh -entry main -profile sm_6_5

struct Vertex
{
    float4 pos : SV_Position;
};

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

//CHECK: OpExecutionMode {{.*}} OutputTrianglesEXT
//CHECK: OpEntryPoint MeshEXT
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
    in uint tig : SV_GroupIndex,
    out vertices Vertex verts[MAX_VERTS],
    out indices uint3 triangles[MAX_PRIMS])
{
    SetMeshOutputCounts(3, 1);

    if (tig < 3)
    {
        Vertex v;
        v.pos = float4(float(tig), 0, 0, 1);
        verts[tig] = v;
    }

    if (tig < 1)
    {
        triangles[tig] = uint3(0, 1, 2);
    }
}
