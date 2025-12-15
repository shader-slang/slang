//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry vertexMain

// Test that vertex-only semantics are validated correctly

struct VertexInput
{
    float3 position : POSITION;
    uint vertexID : SV_VertexID;  // Valid in vertex stage
    uint instanceID : SV_InstanceID;  // Valid in vertex stage
};

struct VertexOutput
{
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1.0);
    return output;
}

// CHECK: vertexMain

