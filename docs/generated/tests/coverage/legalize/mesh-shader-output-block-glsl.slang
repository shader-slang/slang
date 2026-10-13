//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the GLSL mesh-output legalization that redeclares only the used members of gl_MeshPerVertexEXT and gl_MeshPerPrimitiveEXT and routes indices to gl_PrimitiveTriangleIndicesEXT.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `legalizeMeshOutputParam` (slang-ir-glsl-legalize.cpp:3502) on the
// text-GLSL arm. Unlike SPIR-V, where each mesh output becomes an independent
// `OpVariable`, GLSL requires the mesh builtins to live inside the predeclared
// `gl_MeshPerVertexEXT` / `gl_MeshPerPrimitiveEXT` interface blocks. The
// legalizer therefore scans the leaf addresses of the output value, keeps only
// the leaves whose import name is one of the recognised `gl_*` mesh builtins,
// and synthesizes a block redeclaration containing exactly those members in
// declaration order. CHECK asserts that the vertex block contains just the
// three members this shader writes (Position/PointSize/ClipDistance, not the
// whole predeclared block), that the primitive block is `perprimitiveEXT` and
// carries the int-typed gl_PrimitiveID / gl_Layer / gl_ViewportIndex plus the
// bool gl_CullPrimitiveEXT, and that the index output is the separate
// `gl_PrimitiveTriangleIndicesEXT` array rather than a block member.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage mesh

struct Vertex
{
    float4 pos : SV_Position;
    float psize : SV_PointSize;
    float clip[1] : SV_ClipDistance;
}
struct Prim
{
    uint pid : SV_PrimitiveID;
    uint layer : SV_RenderTargetArrayIndex;
    uint vp : SV_ViewportArrayIndex;
    bool cull : SV_CullPrimitive;
}

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
    uint tid : SV_DispatchThreadID,
    OutputIndices<uint3, 1> triangles,
    OutputVertices<Vertex, 3> verts,
    OutputPrimitives<Prim, 1> prims)
{
    SetMeshOutputCounts(3, 1);
    verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
    verts[tid].psize = 1.0;
    verts[tid].clip[0] = 0.5;
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].pid = 42;
        prims[0].layer = 1;
        prims[0].vp = 2;
        prims[0].cull = false;
    }
}

//GLSL: #extension GL_EXT_mesh_shader : require
// Per-vertex block redeclared with exactly the members this shader writes.
//GLSL: out gl_MeshPerVertexEXT
//GLSL-NEXT: {
//GLSL-NEXT: vec4 gl_Position;
//GLSL-NEXT: float gl_PointSize;
//GLSL-NEXT: float  gl_ClipDistance[1];
//GLSL-NEXT: } gl_MeshVerticesEXT[3
// Per-primitive block, tagged perprimitiveEXT; the uint HLSL fields are
// narrowed to the int/bool types GLSL declares for these builtins.
//GLSL: perprimitiveEXT out gl_MeshPerPrimitiveEXT
//GLSL-NEXT: {
//GLSL-NEXT: int gl_PrimitiveID;
//GLSL-NEXT: int gl_Layer;
//GLSL-NEXT: int gl_ViewportIndex;
//GLSL-NEXT: bool gl_CullPrimitiveEXT;
//GLSL-NEXT: } gl_MeshPrimitivesEXT[1
// Indices are a standalone array, not a block member.
//GLSL: out uvec3  gl_PrimitiveTriangleIndicesEXT[1
//GLSL: SetMeshOutputsEXT(3
//GLSL-DAG: gl_MeshVerticesEXT[{{.*}}].gl_Position =
//GLSL-DAG: gl_MeshPrimitivesEXT[{{.*}}].gl_PrimitiveID =
