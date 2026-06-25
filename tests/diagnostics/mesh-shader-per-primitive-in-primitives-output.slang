//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -skip-spirv-validation

// Test that per-primitive semantics in OutputPrimitives (the valid case) do NOT produce errors.
// This is the positive counterpart to mesh-shader-per-primitive-in-vertex-output.slang.
// CHECK: OpCapability

struct VertexOutput
{
    float4 pos : SV_Position;
};

struct PrimOutput
{
    uint primId : SV_PrimitiveID;
    bool cull : SV_CullPrimitive;
    uint layer : SV_RenderTargetArrayIndex;
    uint viewport : SV_ViewportArrayIndex;
    uint rate : SV_ShadingRate;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain(
    OutputVertices<VertexOutput, 9> vertices,
    OutputIndices<uint3, 3> triangles,
    OutputPrimitives<PrimOutput, 3> primitives)
{
    SetMeshOutputCounts(9, 3);
}
