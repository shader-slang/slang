// Test that @location attributes are correctly emitted when a struct is used
// as both vertex shader output and fragment shader input.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -whole-program
// CHECK: @location(0) color_0
// CHECK: @location(1) color_1
// CHECK: @location(0) color_2

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
    output.position = float4(input.position, 0.0f, 1.0f);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0f);
}
