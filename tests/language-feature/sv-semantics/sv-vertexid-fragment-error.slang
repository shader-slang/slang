//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// SV_VertexID is not supported in fragment shader.
// (Note: SV_InstanceID IS supported in fragment - see commit 2407966e8)

// CHECK: error 30703

[shader("fragment")]
float4 main(
    uint vertexID : SV_VertexID)
    : SV_Target
{
    return float4(vertexID, 0, 0, 1);
}
