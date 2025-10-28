//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vertexMain -stage vertex

// Test edge cases for WGSL location attribute generation

struct ComplexInput
{
    float3 position : POSITION;
    float noSemantic; // Field without explicit semantic
    float2 texcoord : TEXCOORD0;
}

struct ComplexOutput  
{
    float4 position : SV_Position;
    float someValue; // Field without explicit semantic  
    float2 texcoord : TEXCOORD0;
}

[shader("vertex")]
ComplexOutput vertexMain(ComplexInput input)
{
    ComplexOutput output;
    output.position = float4(input.position, 1.0);
    output.someValue = input.noSemantic;
    output.texcoord = input.texcoord;
    return output;
}

// CHECK: struct ComplexOutput_0
// CHECK: @builtin(position) position_0 : vec4<f32>,
// CHECK: @location({{[0-9]+}}) someValue_0 : f32,
// CHECK: @location({{[0-9]+}}) texcoord_0 : vec2<f32>,

// CHECK: struct vertexInput_0  
// CHECK: @location({{[0-9]+}}) position_1 : vec3<f32>,
// CHECK: @location({{[0-9]+}}) noSemantic_0 : f32,
// CHECK: @location({{[0-9]+}}) texcoord_1 : vec2<f32>,