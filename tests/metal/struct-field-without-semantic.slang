//TEST:SIMPLE(filecheck=CHECK): -target metal -entry vertexMain -stage vertex

// Test case for issue where WGSL/Metal segfault when compiling shaders with 
// nested structs containing fields without semantics (like originalBitangent)

struct Vertex{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VsOutput{
    Vertex original;
    float3 originalBitangent; // This field has no semantic and was causing the segfault
    float4 svPosition : SV_Position;
}

[shader("vertex")]
VsOutput vertexMain(Vertex input)
{
    VsOutput output;
    output.original = input;
    output.originalBitangent = float3(0, 0, 0);
    output.svPosition = float4(input.position, 1.0);
    return output;
}

// CHECK: vertex
// CHECK: vertexMain
// CHECK: originalBitangent