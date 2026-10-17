//TEST:SIMPLE(filecheck=IMPLICIT): -target wgsl -stage vertex -entry implicitMain
//TEST:SIMPLE(filecheck=EXPLICIT): -target wgsl -stage vertex -entry explicitMain
//TEST:SIMPLE(filecheck=NESTED): -target wgsl -stage vertex -entry nestedMain

// Vertex input locations follow the varying-input layout (declaration order, or explicit
// `[[vk::location]]`), which is what reflection reports and what WebGPU pipelines bind as
// `shaderLocation`. They must not be derived from user semantic indices, which here would put
// `lineWidth` (TEXCOORD0) and `start` (POSITION0) on the same index.

struct ImplicitInput
{
    // IMPLICIT-DAG: @location(0) start{{.*}} : vec3<f32>
    float3 start : POSITION0;
    // IMPLICIT-DAG: @location(1) lineWidth{{.*}} : f32
    float lineWidth : TEXCOORD0;
    // IMPLICIT-DAG: @location(2) end{{.*}} : vec3<f32>
    float3 end : TEXCOORD1;
    // IMPLICIT-DAG: @location(3) color{{.*}} : vec4<f32>
    float4 color : COLOR0;
};

float4 implicitMain(ImplicitInput input) : SV_Position
{
    return float4(input.start + input.end * input.lineWidth, 1.0) + input.color;
}

struct ExplicitInput
{
    // EXPLICIT-DAG: @location(4) position{{.*}} : vec3<f32>
    [[vk::location(4)]] float3 position : POSITION0;
    // EXPLICIT-DAG: @location(1) weight{{.*}} : f32
    [[vk::location(1)]] float weight : TEXCOORD0;
};

float4 explicitMain(ExplicitInput input) : SV_Position
{
    return float4(input.position * input.weight, 1.0);
}

// Nested structs are flattened; locations still follow the (accumulated) layout offsets.
struct NestedWidth
{
    float lineWidth : TEXCOORD0;
    float3 end : TEXCOORD1;
};

struct NestedInput
{
    // NESTED-DAG: @location(0) start{{.*}} : vec3<f32>
    float3 start : POSITION0;
    // NESTED-DAG: @location(1) lineWidth{{.*}} : f32
    // NESTED-DAG: @location(2) end{{.*}} : vec3<f32>
    NestedWidth more;
    // NESTED-DAG: @location(3) color{{.*}} : vec4<f32>
    float4 color : COLOR0;
};

float4 nestedMain(NestedInput input) : SV_Position
{
    return float4(input.start + input.more.end * input.more.lineWidth, 1.0) + input.color;
}
