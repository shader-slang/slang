// Using SV_VertexID in a fragment shader should produce a proper diagnostic error,
// not an internal compiler error. SV_VertexID is only valid in vertex shaders.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -skip-spirv-validation

struct VsOutput
{
    float3 position;
    float4 screen_pos : SV_Position;
};

[shader("vertex")]
VsOutput vs(uint vertex_id : SV_VertexID)
{
    VsOutput output;
    output.position = float3(vertex_id, 0, 0);
    output.screen_pos = float4(output.position, 1.0);
    return output;
}

// CHECK-NOT: error 99999
// CHECK-NOT: internal error
// CHECK: error {{.*}}SV_VertexID
[shader("fragment")]
float4 fs(VsOutput input, uint vertex_id : SV_VertexID) : SV_Target
{
    return float4(input.position, 1.0) + vertex_id;
}
