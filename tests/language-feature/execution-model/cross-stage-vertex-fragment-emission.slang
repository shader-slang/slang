// Test cross-stage data flow from vertex shader to fragment shader.
// Verifies correct emission of vertex outputs that become fragment inputs,
// including struct-based varying data and system value semantics.
//
// This exercises the rasterization pipeline model from the docs:
// "Per-invocation inputs for the fragment shader come from the rasterizer
//  and the vertex shader stage."

//TEST:SIMPLE(filecheck=CHECK_VERT): -target hlsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FRAG): -target hlsl -entry fragmentMain -stage fragment

struct VertexOutput
{
    float3 color    : COLOR;
    float2 texcoord : TEXCOORD0;
    float4 position : SV_Position;
};

struct FragmentOutput
{
    float4 color : SV_Target;
};

// Vertex shader: transform position and pass through varying data
// CHECK_VERT: COLOR
// CHECK_VERT: TEXCOORD
// CHECK_VERT: SV_Position
[shader("vertex")]
VertexOutput vertexMain(float3 pos : POSITION, float3 col : COLOR, float2 uv : TEXCOORD0)
{
    VertexOutput output;
    output.position = float4(pos, 1.0);
    output.color = col;
    output.texcoord = uv;
    return output;
}

// Fragment shader: consume the vertex outputs
// CHECK_FRAG: SV_Target
// CHECK_FRAG: COLOR
// CHECK_FRAG: TEXCOORD
[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;
    output.color = float4(input.color * input.texcoord.x, 1.0);
    return output;
}
