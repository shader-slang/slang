//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpMatrixTimesVector

struct Vertex
{
    float4x4 m;
    float4 pos;
}

[shader("vertex")]
float4 vertMain(Vertex v) : SV_Position
{
    return mul(v.m, v.pos);
}