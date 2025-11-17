//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g3 -O0 -emit-spirv-directly

// Structure for shader parameters
struct ShaderParameters
{
    float4 fillColor;
    float edgeThickness;
    float4 edgeColor;
};

// Constant buffer
cbuffer ShaderConstants : register(b0)
{
    ShaderParameters params;
};

// Input from vertex shader
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// Output to render target
struct PSOutput
{
    float4 color : SV_TARGET;
};

// Main entry point
PSOutput main(VSOutput input)
{
    // Calculate distance from center for a circle effect
    float2 center = float2(0.5, 0.5);
    float2 uv = input.texCoord - center;
    float distance = length(uv);

    // Create circular shape with edge
    float circle = 1.0 - step(0.4, distance);         // Circle with radius 0.4
    float edge = step(0.4 - params.edgeThickness, distance) * circle; // Edge of the circle

    // Combine fill and edge colors
    float4 fillPart = circle * (1.0 - edge) * params.fillColor;
    float4 edgePart = edge * params.edgeColor;

    // Create final color
    PSOutput output;
    output.color = float4(1, 1, 1, 1);

    return output;
}

// Check that DebugTypeMember instructions for ShaderParameters_std140 have correct line numbers
// CHECK: OpString "fillColor"
// CHECK: OpString "edgeThickness"
// CHECK: OpString "edgeColor"
// CHECK: DebugTypeMember %{{.*}} %{{.*}} %{{.*}} %uint_6
// CHECK: DebugTypeMember %{{.*}} %{{.*}} %{{.*}} %uint_7
// CHECK: DebugTypeMember %{{.*}} %{{.*}} %{{.*}} %uint_8
