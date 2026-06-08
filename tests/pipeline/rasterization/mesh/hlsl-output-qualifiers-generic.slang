//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage mesh -entry meshMain

struct Vertex   { float4 pos : SV_Position; }
struct Prim     { uint  pid : SV_PrimitiveID; }

[shader("mesh")][outputtopology("triangle")][numthreads(3,1,1)]
void meshMain(
    uint tid : SV_DispatchThreadID,
     OutputIndices<uint3, 1>   triangles,
     OutputVertices<Vertex, 3> verts,
     OutputPrimitives<Prim, 1> prims)
{
    SetMeshOutputCounts(3, 1);
    if (tid < 3)
    {
        verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
    }
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].pid = 42;
    }
}

// CHECK-DAG: out vertices   {{[A-Za-z_0-9]+}} verts
// CHECK-DAG: out indices    {{[A-Za-z_0-9]+}} triangles
// CHECK-DAG: out primitives {{[A-Za-z_0-9]+}} prims
// CHECK-NOT: out indices    {{.*}} out
// CHECK-NOT: out vertices   {{.*}} out
// CHECK-NOT: out primitives {{.*}} out
