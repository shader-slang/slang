//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage vertex

// Test case for GitHub issue #8882
// When [shader("vertex")] attribute is missing but stage is specified via command line,
// system-value semantics like SV_VertexId in struct parameters should still work correctly.

// CHECK-DAG: BuiltIn VertexIndex
// CHECK-DAG: BuiltIn BaseVertex
// CHECK-NOT: OpAccessChain {{.*}} %gl_VertexIndex

struct Input
{
    uint VertexId : SV_VertexId;
};

struct Output
{
    float4 Position : SV_Position;
    float2 Uv : Uv;
};

// Note: [shader("vertex")] is intentionally missing - stage is specified via -stage vertex
void main(in Input input, out Output output)
{
    float4 positionUvs[3];
    positionUvs[0] = float4(-1, -1, 0, 0);
    positionUvs[1] = float4(3, -1, 2, 0);
    positionUvs[2] = float4(-1, 3, 0, 2);

    output.Position = float4(positionUvs[input.VertexId].xy, 0, 1);
    output.Uv = float2(positionUvs[input.VertexId].zw);
}

