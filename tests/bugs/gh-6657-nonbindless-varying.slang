// TEST:REFLECTION(filecheck=CHECK):-target spirv -stage vertex -entry vertexMain
// TEST:REFLECTION(filecheck=CHECK):-target hlsl -stage vertex -entry vertexMain -profile sm_6_6
//TEST:REFLECTION(filecheck=CHECK):-target glsl -stage vertex -entry vertexMain
//TEST:REFLECTION(filecheck=CHECK):-target wgsl -stage vertex -entry vertexMain

// Test that DescriptorHandle<T> as a varying input is correctly reflected
// on non-bindless targets (SPIRV/HLSL/GLSL/WGSL)
//
// Type should be DescriptorHandle
// Layout should match uint2 (occupies 1 varying location)

// Verify that the DescriptorHandle occupies location 3
// CHECK: "name": "texture_handle"
// CHECK: "kind": "struct"
// CHECK: "name": "DescriptorHandle"
// CHECK: "kind": "varyingInput"
// CHECK: "index": 3

// Verify that the followingData occupies location 4
// CHECK: "name": "followingData"
// CHECK: "kind": "varyingInput"
// CHECK: "index": 4

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    Texture2D.Handle texture_handle;
    uint2 followingData;
}

float4 vertexMain(Vertex vertex) : SV_Position
{
    float4 result = float4(vertex.position, 1.0);
    result.xyz += vertex.normal * 0.1;
    result.xy += vertex.uv * 0.1;
    uint2 handle = (uint2)vertex.texture_handle;
    result.x += float(handle.x + vertex.followingData.x) * 0.0001;
    return result;
}
