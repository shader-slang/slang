//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry vertexMain -stage vertex

// Test that user-defined semantics (like COLOR) in vertex shader output
// structs get proper @location attributes in WGSL output.

// WGSL-LABEL: struct VertexOutput
// WGSL: @builtin(position){{.*}}position
// WGSL: @location(0){{.*}}color

struct VertexInput
{
    float2 position : POSITION;
    float3 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) 
{
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}
