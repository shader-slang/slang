//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#where-the-metal-specific-opcodes-come-from
//META: doc_section_digest=a3fdb204a916a518fb6e90d1b9ab54f11e343e46db0c160597f5ad1e2258a551
//META: purpose=Mesh-shader output writes lower to the Metal-only `metalSetVertex` / `metalSetIndices` opcodes, which the emitter renders as `_slang_mesh.set_vertex(...)` / `_slang_mesh.set_index(...)`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core-module route for the Metal-only mesh opcodes: the
// `case metal:` arm of the output-type subscript setter calls an
// `__intrinsic_op` helper, so `verts[i] = v` lowers straight to the opcode
// rather than going through an IR pass. CHECK asserts the entry point
// takes a `metal::mesh<...>` argument named `_slang_mesh` and that the
// vertex and index writes emit as its member calls.

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

// CHECK: metal::mesh<
// CHECK-SAME: _slang_mesh
// CHECK-DAG: _slang_mesh.set_vertex(
// CHECK-DAG: _slang_mesh.set_index(
