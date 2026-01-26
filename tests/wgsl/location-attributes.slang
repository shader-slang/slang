//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vertexMain -stage vertex

// Test to ensure WGSL location attributes are generated correctly for varying parameters

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
    float3 color : COLOR0;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
    float3 color : COLOR0;
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1.0);
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}

// CHECK: struct VertexOutput_0
// CHECK: @builtin(position) position_0 : vec4<f32>,
// CHECK: @location({{[0-9]+}}) normal_0 : vec3<f32>,
// CHECK: @location({{[0-9]+}}) texcoord_0 : vec2<f32>,
// CHECK: @location({{[0-9]+}}) color_0 : vec3<f32>,

// CHECK: struct vertexInput_0
// CHECK: @location({{[0-9]+}}) position_1 : vec3<f32>,
// CHECK: @location({{[0-9]+}}) normal_1 : vec3<f32>,
// CHECK: @location({{[0-9]+}}) texcoord_1 : vec2<f32>,
// CHECK: @location({{[0-9]+}}) color_1 : vec3<f32>,