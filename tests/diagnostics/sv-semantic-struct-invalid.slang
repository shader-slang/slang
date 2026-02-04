//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that system value semantic validation works for struct fields.
// SV_ semantics inside structs should be validated for correct stage and direction.

// Test 1: Output struct with input-only semantic field
// SV_VertexID is input-only in vertex shader, cannot be in output struct
struct VertexOutput
{
    float4 pos : SV_Position;
    // CHECK: ([[# @LINE+1]]): error 30702: system value semantic 'SV_VertexID' cannot be used as output in 'vertex' shader stage
    uint vertexId : SV_VertexID;
};

[shader("vertex")]
VertexOutput vsMain(uint id : SV_VertexID)
{
    VertexOutput output;
    output.pos = float4(0, 0, 0, 1);
    output.vertexId = id;
    return output;
}

// Test 2: Input struct with output-only semantic field
// SV_Depth is output-only in fragment shader, cannot be in input struct
struct FragmentInput
{
    float4 pos : SV_Position;
    // CHECK: ([[# @LINE+1]]): error 30702: system value semantic 'SV_Depth' cannot be used as input in 'pixel' shader stage
    float depth : SV_Depth;
};

[shader("fragment")]
float4 fsMain(FragmentInput input) : SV_Target
{
    return input.pos;
}
