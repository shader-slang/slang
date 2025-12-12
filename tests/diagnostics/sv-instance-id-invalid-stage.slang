//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry fs

// Test that SV_InstanceID is properly rejected in non-vertex shader stages

struct VsOutput {
    float4 position : SV_Position;
}

// CHECK: error 38028
// CHECK: semantic 'SV_InstanceID' is not available in pixel shader stage

[shader("fragment")]
float4 fs(VsOutput input, uint instance_id: SV_InstanceID) : SV_Target
{
    return float4(instance_id, 0, 0, 1);
}
