//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry main
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile sm_6_6 -entry main

//CHECK_HLSL: void main(
//CHECK_HLSL: in payload Payload
//CHECK_HLSL: out vertices Vertex
//CHECK_HLSL: out indices uint3
//CHECK_HLSL: out primitives Primitive
//DXIL: define void @

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
