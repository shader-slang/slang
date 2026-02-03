//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vertexMain -stage vertex

// Test that ParameterBlock<StructuredBuffer<T>> generates proper @binding and @group attributes in WGSL.
// This is a regression test for https://github.com/shader-slang/slang/issues/8358

struct TriangleData {
    float3[3] position;
    uint32_t color;
}

ParameterBlock<StructuredBuffer<TriangleData>> triangle_data;

// CHECK: @binding(0) @group(0) var<storage, read> triangle_data

[shader("vertex")]
float4 vertexMain(uint vIdx : SV_VertexID, uint iIdx : SV_InstanceID) : SV_Position
{
    return float4(triangle_data[iIdx].position[vIdx], 1.0);
}
