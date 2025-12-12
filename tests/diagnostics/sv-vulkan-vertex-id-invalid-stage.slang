//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry fs

// Test that SV_VulkanVertexID is properly rejected in non-vertex shader stages
// This semantic maps to VertexIndex builtin in SPIR-V which is only valid in vertex shaders

struct VsOutput {
    float4 position : SV_Position;
}

// CHECK: error 38028
// CHECK: semantic 'SV_VulkanVertexID' is not available in pixel shader stage

[shader("fragment")]
float4 fs(VsOutput input, uint vertex_id: SV_VulkanVertexID) : SV_Target
{
    return float4(vertex_id, 0, 0, 1);
}
