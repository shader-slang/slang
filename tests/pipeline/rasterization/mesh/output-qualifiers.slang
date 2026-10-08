// Regression for shader-slang/slang#11543.
//
// HLSL mesh-shader output qualifiers (`out vertices`/`out indices`/`out primitives`)
// must be emitted exactly once for BOTH the generic `OutputVertices<T,N>` family and
// the HLSL-style `out vertices T[N]` spelling. Two bugs are pinned here:
//   * generic outputs whose element struct is driven only by system-value semantics
//     (e.g. `{ float4 pos : SV_Position; }`) used to emit no qualifier at all; and
//   * the HLSL-style spelling used to emit a doubled `out` (e.g. `out indices out`),
//     which DXC rejects.

//TEST:SIMPLE(filecheck=GENERIC):-target hlsl -stage mesh -entry meshGeneric
//TEST:SIMPLE(filecheck=HLSLSTYLE):-target hlsl -stage mesh -entry meshHlslStyle
// Compile the emitted HLSL through DXC for both spellings: a doubled `out`
// (the pre-fix bug) is invalid HLSL and is rejected by DXC, so a successful
// dxil compile is itself the regression check.
//TEST:SIMPLE(filecheck=DXIL_GEN):-target dxil-assembly -stage mesh -entry meshGeneric -profile sm_6_6
//TEST:SIMPLE(filecheck=DXIL_HLSL):-target dxil-assembly -stage mesh -entry meshHlslStyle -profile sm_6_6

struct Vertex { float4 pos : SV_Position; }
struct Prim   { uint   pid : SV_PrimitiveID; }

// Generic mesh-output spelling: the element structs carry only system-value semantics.
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void meshGeneric(
    uint tid : SV_DispatchThreadID,
    OutputIndices<uint3, 1>   triangles,
    OutputVertices<Vertex, 3> verts,
    OutputPrimitives<Prim, 1> prims)
{
    SetMeshOutputCounts(3, 1);
    if (tid < 3)
        verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].pid = 42;
    }
}

// HLSL-style mesh-output spelling (same outputs, declared with the `out` modifier).
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void meshHlslStyle(
    uint tid : SV_DispatchThreadID,
    out indices    uint3  triangles[1],
    out vertices   Vertex verts[3],
    out primitives Prim   prims[1])
{
    SetMeshOutputCounts(3, 1);
    if (tid < 3)
        verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].pid = 42;
    }
}

// Each qualifier appears once on the signature; none is dropped, and none is doubled
// (a doubled qualifier would read as e.g. `out indices out`).
// GENERIC: void meshGeneric
// GENERIC-DAG: out indices {{[A-Za-z_0-9]+}} triangles
// GENERIC-DAG: out vertices {{[A-Za-z_0-9]+}} verts
// GENERIC-DAG: out primitives {{[A-Za-z_0-9]+}} prims
// GENERIC-NOT: indices out
// GENERIC-NOT: vertices out
// GENERIC-NOT: primitives out
// GENERIC: SetMeshOutputCounts

// HLSLSTYLE: void meshHlslStyle
// HLSLSTYLE-DAG: out indices {{[A-Za-z_0-9]+}} triangles
// HLSLSTYLE-DAG: out vertices {{[A-Za-z_0-9]+}} verts
// HLSLSTYLE-DAG: out primitives {{[A-Za-z_0-9]+}} prims
// HLSLSTYLE-NOT: indices out
// HLSLSTYLE-NOT: vertices out
// HLSLSTYLE-NOT: primitives out
// HLSLSTYLE: SetMeshOutputCounts

// DXC accepts the emitted HLSL for both spellings (the mesh entry compiles to DXIL).
// DXIL_GEN: call void @dx.op.setMeshOutputCounts
// DXIL_GEN: @dx.op.emitIndices
// DXIL_HLSL: call void @dx.op.setMeshOutputCounts
// DXIL_HLSL: @dx.op.emitIndices
