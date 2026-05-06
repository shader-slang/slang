// Mesh shader entry point emission.
//
// Mesh shaders write to two output arrays — one for vertices, one for
// primitives — and call SetMeshOutputCounts(numVertices, numPrimitives)
// to declare the produced topology. Each per-vertex / per-primitive
// output array must be `out` and sized at compile time per the diagnostic
// tests in tests/diagnostics/execution-model/mesh-output-*.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage mesh -entry meshMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage mesh -entry meshMain -emit-spirv-directly

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
void meshMain(
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

// HLSL: keeps the explicit attributes and output declarations.
// CHECK_HLSL-DAG: outputtopology("triangle")
// CHECK_HLSL-DAG: numthreads(3, 1, 1)
// CHECK_HLSL-DAG: out indices
// CHECK_HLSL-DAG: out vertices
// CHECK_HLSL-DAG: SetMeshOutputCounts

// SPIRV: emits MeshEXT execution mode + the per-vertex / per-primitive
// BuiltIn decorations. Slang lowers the output arrays through the
// VK_EXT_mesh_shader extension.
// CHECK_SPIRV-DAG: MeshEXT
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} OutputVertices
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} OutputPrimitivesEXT
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} OutputTrianglesEXT
// CHECK_SPIRV-DAG: BuiltIn Position
// CHECK_SPIRV-DAG: BuiltIn PrimitiveTriangleIndicesEXT
// CHECK_SPIRV-DAG: OpSetMeshOutputsEXT
