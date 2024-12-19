//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpVectorTimesMatrix

struct Vertex
{
    float4x4 m;
    float4 pos;
}

struct VertexOut
{
    float4 pos : SV_Position;
    float4 color;
}

[shader("vertex")]
VertexOut vertMain(Vertex v)
{
    VertexOut o;
    o.pos = mul(v.m, v.pos);
    o.color = v.pos;
    return o;
}