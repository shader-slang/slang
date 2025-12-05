//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry vertexMain -emit-spirv-directly -stage vertex

// This test verifies that DescriptorHandle is properly decomposed to uint2 for location assignment
// and doesn't cause overlapping location assignments.

// CHECK: OpDecorate %{{.*}}position Location 0
// CHECK: OpDecorate %{{.*}}normal Location 1  
// CHECK: OpDecorate %{{.*}}uv Location 2
// CHECK: OpDecorate %{{.*}}not_decomposed Location 3
// CHECK: OpDecorate %{{.*}}explicit_handle Location 4

struct Vertex
{
    float3 position;
    float3 normal; 
    float2 uv;
    Texture2D.Handle not_decomposed; // Should decompose to uint2 and get location 3
    uint2 explicit_handle; // Workaround - explicit uint2, should get location 4
}

float4 vertexMain(Vertex vertex) : SV_Position
{
    float4 result = float4(vertex.position, 1.0);
    result.xyz += vertex.normal * 0.1;
    result.xy += vertex.uv * 0.1;

    uint2 handle1 = (uint2)vertex.not_decomposed;
    uint2 handle2 = vertex.explicit_handle;
    result.x += float(handle1.x + handle2.x) * 0.0001;

    return result;
}
