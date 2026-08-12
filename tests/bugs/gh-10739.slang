//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry main
//TEST:SIMPLE(filecheck=CHECK_HLSL_LEGACY): -target hlsl -entry main_legacy
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile sm_6_6 -entry main
//TEST:SIMPLE(filecheck=DXIL_LEGACY): -target dxil -profile sm_6_6 -entry main_legacy

//CHECK_HLSL: void main(
//CHECK_HLSL: in payload Payload
//CHECK_HLSL: out vertices Vertex
//CHECK_HLSL: out indices uint3
//CHECK_HLSL: out primitives Primitive
//CHECK_HLSL-NOT: out vertices out
//CHECK_HLSL-NOT: out indices out
//CHECK_HLSL-NOT: out primitives out
//DXIL: define void @main(

//CHECK_HLSL_LEGACY: void main_legacy(
//CHECK_HLSL_LEGACY: in payload Payload
//CHECK_HLSL_LEGACY: out vertices Vertex
//CHECK_HLSL_LEGACY: out indices uint3
//CHECK_HLSL_LEGACY: out primitives Primitive
//CHECK_HLSL_LEGACY-NOT: out vertices out
//CHECK_HLSL_LEGACY-NOT: out indices out
//CHECK_HLSL_LEGACY-NOT: out primitives out
//DXIL_LEGACY: define void @main_legacy(

struct Payload
{
    uint primitiveTag;
};

struct Vertex
{
    float4 pos : SV_Position;
    float3 color : Color;
};

struct Primitive
{
    nointerpolation uint primitiveTag : PrimitiveTag;
};

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void main(
    in uint tig : SV_GroupIndex,
    in payload Payload payload,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> tris,
    OutputPrimitives<Primitive, MAX_PRIMS> prims)
{
    SetMeshOutputCounts(3, 1);

    if (tig < 3)
    {
        const float x = tig == 1 ? 0.5 : (tig == 2 ? -0.5 : 0.0);
        const float y = tig == 0 ? -0.5 : 0.5;
        verts[tig] = { float4(x, y, 0.0, 1.0), float3(1.0, 0.0, 0.0) };
    }

    if (tig < 1)
    {
        tris[0] = uint3(0, 1, 2);
        prims[0].primitiveTag = payload.primitiveTag;
    }
}

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void main_legacy(
    in uint tig : SV_GroupIndex,
    in payload Payload payload,
    out vertices Vertex verts[MAX_VERTS],
    out indices uint3 tris[MAX_PRIMS],
    out primitives Primitive prims[MAX_PRIMS])
{
    SetMeshOutputCounts(3, 1);

    if (tig < 3)
    {
        const float x = tig == 1 ? 0.5 : (tig == 2 ? -0.5 : 0.0);
        const float y = tig == 0 ? -0.5 : 0.5;
        verts[tig] = { float4(x, y, 0.0, 1.0), float3(1.0, 0.0, 0.0) };
    }

    if (tig < 1)
    {
        tris[0] = uint3(0, 1, 2);
        prims[0].primitiveTag = payload.primitiveTag;
    }
}
