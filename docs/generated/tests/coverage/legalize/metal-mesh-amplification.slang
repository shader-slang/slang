//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the Metal legalization of the amplification and mesh stages, rewriting DispatchMesh into a synthesized object_data payload parameter plus mesh_grid_properties.set_threadgroups_per_grid, and the mesh outputs into a metal::mesh object driven by set_vertex/set_index/set_primitive.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `LegalizeMetalEntryPointContext::legalizeAmplificationStageEntryPoint`
// and `legalizeMeshStageEntryPoint`
// (slang-ir-legalize-varying-params.cpp:4530 and :4597). Metal's mesh
// pipeline does not resemble HLSL's: an amplification shader is an
// `[[object]]` function that writes its payload through an `object_data`
// pointer and launches the mesh grid through a `mesh_grid_properties`
// object, and neither of those exists as a parameter in the user's source.
// The amplification legalizer therefore finds the `DispatchMesh` call and
// synthesizes both parameters, naming them `_slang_mesh_payload` and
// `_slang_mgp`, before rewriting the call into
// `set_threadgroups_per_grid`. The mesh legalizer does the mirror-image job
// on the consumer side, re-typing the `payload` parameter as an
// `object_data` pointer and turning the output arrays into one
// `metal::mesh<...>` object. CHECK asserts the synthesized names and the
// rewritten call on the amplification side, and the `metal::mesh` parameter
// with its `set_*` calls on the mesh side.
//TEST:SIMPLE(filecheck=AMP):-target metal -entry ampMain -stage amplification
//TEST:SIMPLE(filecheck=MESH):-target metal -entry meshMain -stage mesh

struct Payload { uint count; }
struct Vertex { float4 pos : SV_Position; }
struct Prim { uint pid : SV_PrimitiveID; }

[shader("amplification")]
[numthreads(1, 1, 1)]
void ampMain(uint3 gid : SV_GroupID)
{
    Payload p;
    p.count = gid.x;
    DispatchMesh(1, 1, 1, p);
}

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void meshMain(
    uint tid : SV_DispatchThreadID,
    in payload Payload p,
    OutputIndices<uint3, 1> triangles,
    OutputVertices<Vertex, 3> verts,
    OutputPrimitives<Prim, 1> prims)
{
    SetMeshOutputCounts(3, 1);
    Vertex v;
    v.pos = float4(float(p.count), 0, 0, 1);
    verts[tid] = v;
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        Prim pr;
        pr.pid = 7;
        prims[0] = pr;
    }
}

// The amplification entry point becomes [[object]] and gains two parameters
// that do not exist in the source: the payload pointer and the grid object.
//AMP: {{\[\[}}object]] void ampMain(uint3 gid_0 {{\[\[}}threadgroup_position_in_grid]], Payload_0 object_data* _slang_mesh_payload {{\[\[}}payload]], mesh_grid_properties  _slang_mgp)
// DispatchMesh becomes a payload store followed by the grid launch.
//AMP: *_slang_mesh_payload = *(&p_0); _slang_mgp.set_threadgroups_per_grid(uint3((1U), (1U), (1U)));

// On the mesh side the payload parameter is re-typed as an object_data
// pointer and the three output arrays collapse into one metal::mesh object.
//MESH: {{\[\[}}mesh]] void meshMain(uint3 tid_0 {{\[\[}}thread_position_in_grid]], const Payload_0 object_data* p_0 {{\[\[}}payload]], metal::mesh<Vertex_0, Prim_0, 3U, 1U, metal::topology::triangle> _slang_mesh)
//MESH-DAG: _slang_mesh.set_vertex(
//MESH-DAG: _slang_mesh.set_index(
//MESH-DAG: _slang_mesh.set_primitive(
