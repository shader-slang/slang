// Test that @location attributes are correctly emitted when a struct is used
// as both vertex shader output and fragment shader input.
// Regression test for https://github.com/shader-slang/slang/issues/9396

// This test requires whole-program compilation to trigger the bug where semantic
// decorations were incorrectly removed from VertexOutput when it was hoisted as a
// fragment shader input parameter. The wgsl-spirv-asm target triggers whole-program
// mode, and tint will validate the WGSL output (failing on missing @location).
//TEST:SIMPLE(filecheck=CHECK):-target wgsl-spirv-asm

// Check that we get valid SPIRV output (tint compiled the WGSL successfully)
// CHECK: OpEntryPoint Vertex %vertexMain
// CHECK: OpEntryPoint Fragment %fragmentMain

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
