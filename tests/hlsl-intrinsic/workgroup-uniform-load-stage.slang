// `workgroupUniformLoad` synchronizes only in shader stages that have a
// workgroup. A mesh shader gets the two barriers. A fragment or vertex shader
// gets a plain load, because a workgroup control barrier is invalid there
// (SPIR-V VUID-StandaloneSpirv-OpControlBarrier-04682).

//TEST:SIMPLE(filecheck=MESH_SPIRV): -target spirv-asm -stage mesh -entry meshMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=MESH_HLSL): -target hlsl -stage mesh -entry meshMain
//TEST:SIMPLE(filecheck=FRAG_SPIRV): -target spirv-asm -stage fragment -entry fragmentMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=FRAG_HLSL): -target hlsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=VERT_METAL): -target metal -stage vertex -entry vertexMain

groupshared uint sharedVal;

struct MeshVertex
{
    float4 pos : SV_Position;
};

[shader("mesh")]
[OutputTopology("triangle")]
[numthreads(3, 1, 1)]
void meshMain(uint gi: SV_GroupIndex, out indices uint3 triangles[1], out vertices MeshVertex verts[3])
{
    if (gi == 0)
        sharedVal = 3;
    const uint n = workgroupUniformLoad(sharedVal);
    SetMeshOutputCounts(n, 1);
    if (gi < 3)
        verts[gi].pos = float4(float(gi), 0.0, 0.0, 1.0);
    if (gi == 0)
        triangles[0] = uint3(0, 1, 2);
}

// MESH_SPIRV: OpControlBarrier %uint_2 %uint_2 %uint_264
// MESH_SPIRV-NEXT: OpLoad %uint %sharedVal
// MESH_SPIRV-NEXT: OpControlBarrier %uint_2 %uint_2 %uint_264

// MESH_HLSL: GroupMemoryBarrierWithGroupSync()
// MESH_HLSL-NOT: GroupMemoryBarrierWithGroupSync
// MESH_HLSL: = sharedVal
// MESH_HLSL: GroupMemoryBarrierWithGroupSync()

[shader("fragment")]
float4 fragmentMain(float4 pos: SV_Position) : SV_Target
{
    float x = pos.x;
    return float4(workgroupUniformLoad(x), 0, 0, 1);
}

// FRAG_SPIRV: OpEntryPoint Fragment
// FRAG_SPIRV-NOT: OpControlBarrier

// FRAG_HLSL: fragmentMain
// FRAG_HLSL-NOT: GroupMemoryBarrierWithGroupSync

[shader("vertex")]
float4 vertexMain(uint vid: SV_VertexID) : SV_Position
{
    return float4(float(workgroupUniformLoad(vid)), 0, 0, 1);
}

// VERT_METAL: vertexMain
// VERT_METAL-NOT: threadgroup_barrier
