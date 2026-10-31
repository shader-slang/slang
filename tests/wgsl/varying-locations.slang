//TEST:SIMPLE(filecheck=IMPLICIT): -target wgsl -stage vertex -entry implicitMain
//TEST:SIMPLE(filecheck=EXPLICIT): -target wgsl -stage vertex -entry explicitMain
//TEST:SIMPLE(filecheck=NESTED): -target wgsl -stage vertex -entry nestedMain
//TEST:SIMPLE(filecheck=SPARSE): -target wgsl -stage vertex -entry sparseMain
//TEST:SIMPLE(filecheck=PARAMS): -target wgsl -stage vertex -entry paramsMain
//TEST:SIMPLE(filecheck=VOUT): -target wgsl -stage vertex -entry voutMain
//TEST:SIMPLE(filecheck=FRAG): -target wgsl -stage fragment -entry fragMain
//TEST:SIMPLE(filecheck=FRAG_SPIRV): -target spirv-asm -stage fragment -entry fragMain
//TEST:REFLECTION(filecheck=REFLECT): -target wgsl -stage vertex -entry nestedMain -no-codegen
//TEST:SIMPLE(filecheck=MEXPLICIT): -target metal -stage vertex -entry explicitMain
//TEST:SIMPLE(filecheck=MNESTED): -target metal -stage vertex -entry nestedMain
//TEST:SIMPLE(filecheck=MSPARSE): -target metal -stage vertex -entry sparseMain

// WGSL varying `@location`s are the locations computed by layout (declaration order, explicit
// `[[vk::location]]`, or `SV_Target<N>`), which reflection reports and SPIR-V uses. User
// semantic names and indices do not affect them.

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
    // MEXPLICIT-DAG: position{{.*}}attribute(4)
    [[vk::location(4)]] float3 position : POSITION0;
    // EXPLICIT-DAG: @location(1) weight{{.*}} : f32
    // MEXPLICIT-DAG: weight{{.*}}attribute(1)
    [[vk::location(1)]] float weight : TEXCOORD0;
};

float4 explicitMain(ExplicitInput input) : SV_Position
{
    return float4(input.position * input.weight, 1.0);
}

// Nested structs are flattened; each leaf keeps its absolute layout location, and system values
// inside the nested struct take no location.
struct NestedWidth
{
    float lineWidth : TEXCOORD0;
    uint instance : SV_InstanceID;
    float3 end : TEXCOORD1;
};

struct NestedInput
{
    uint vertex : SV_VertexID;
    // NESTED-DAG: @location(0) start{{.*}} : vec3<f32>
    // MNESTED-DAG: start{{.*}}attribute(0)
    float3 start : POSITION0;
    // NESTED-DAG: @location(1) lineWidth{{.*}} : f32
    // NESTED-DAG: @builtin(instance_index) instance{{.*}} : u32
    // NESTED-DAG: @location(2) end{{.*}} : vec3<f32>
    // MNESTED-DAG: lineWidth{{.*}}attribute(1)
    // MNESTED-DAG: end{{.*}}attribute(2)
    NestedWidth more;
    // NESTED-DAG: @location(3) color{{.*}} : vec4<f32>
    // MNESTED-DAG: color{{.*}}attribute(3)
    float4 color : COLOR0;
};

// Reflection reports `more` at 1 and its leaves relative to it: 1 + 0 and 1 + 1.
// REFLECT: "name": "start"
// REFLECT: "kind": "varyingInput", "index": 0
// REFLECT: "name": "more"
// REFLECT: "kind": "varyingInput", "index": 1
// REFLECT: "name": "lineWidth"
// REFLECT: "kind": "varyingInput", "index": 0
// REFLECT: "name": "end"
// REFLECT: "kind": "varyingInput", "index": 1
// REFLECT: "name": "color"
// REFLECT: "kind": "varyingInput", "index": 3

float4 nestedMain(NestedInput input) : SV_Position
{
    return float4(input.start + input.more.end * input.more.lineWidth, 1.0) + input.color +
           float(input.vertex + input.more.instance);
}

struct SparseInput
{
    // SPARSE-DAG: @location(0) a{{.*}} : vec2<f32>
    // MSPARSE-DAG: a{{.*}}attribute(0)
    float2 a : TEXCOORD5;
    // SPARSE-DAG: @location(1) b{{.*}} : vec4<f32>
    // MSPARSE-DAG: b{{.*}}attribute(1)
    float4 b : TEXCOORD2;
};

float4 sparseMain(SparseInput input) : SV_Position
{
    return float4(input.a, 0.0, 1.0) + input.b;
}

// PARAMS-DAG: @location(0) start{{.*}} : vec3<f32>
// PARAMS-DAG: @location(1) lineWidth{{.*}} : f32
// PARAMS-DAG: @location(2) end{{.*}} : vec3<f32>
float4 paramsMain(
    float3 start : POSITION0,
    float lineWidth : TEXCOORD0,
    float3 end : TEXCOORD1,
    uint vertex : SV_VertexID) : SV_Position
{
    return float4(start + end * lineWidth + float(vertex), 1.0);
}

struct Varyings
{
    float4 position : SV_Position;
    // VOUT-DAG: @location(0) uv{{.*}} : vec2<f32>
    // FRAG-DAG: @location(0) uv{{.*}} : vec2<f32>
    float2 uv : TEXCOORD3;
    // VOUT-DAG: @location(1) color{{.*}} : vec4<f32>
    // FRAG-DAG: @location(1) color{{.*}} : vec4<f32>
    float4 color : COLOR0;
    // VOUT-DAG: @location(2) weight{{.*}} : f32
    // FRAG-DAG: @location(2) weight{{.*}} : f32
    float weight : TEXCOORD0;
};

Varyings voutMain(float3 position : POSITION)
{
    Varyings output;
    output.position = float4(position, 1.0);
    output.uv = position.xy;
    output.color = position.xyzz;
    output.weight = position.z;
    return output;
}

struct Targets
{
    // FRAG-DAG: @location(1) second{{.*}} : vec4<f32>
    // FRAG_SPIRV-DAG: OpDecorate %{{.*}}second Location 1
    float4 second : SV_Target1;
    // FRAG-DAG: @location(0) first{{.*}} : vec4<f32>
    // FRAG_SPIRV-DAG: OpDecorate %{{.*}}first Location 0
    float4 first : SV_Target0;
};

Targets fragMain(Varyings input)
{
    Targets output;
    output.second = input.color * input.weight;
    output.first = float4(input.uv, 0.0, 1.0);
    return output;
}
