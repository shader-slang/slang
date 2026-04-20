// Test cross-stage vertex/fragment emission with shared interpolant semantics.

//TEST:SIMPLE(filecheck=CHECK_VERT): -target hlsl -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FRAG): -target hlsl -entry fragMain -stage fragment

struct VsOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

// CHECK_VERT: SV_Position
// CHECK_VERT: COLOR
// CHECK_VERT: TEXCOORD
[shader("vertex")]
VsOut vertMain(uint vid : SV_VertexID)
{
    VsOut o;
    o.pos = float4(float(vid), 0.0, 0.0, 1.0);
    o.color = float4(1.0, 0.0, 0.0, 1.0);
    o.uv = float2(float(vid), 0.0);
    return o;
}

// CHECK_FRAG: SV_TARGET
[shader("fragment")]
float4 fragMain(float4 color : COLOR, float2 uv : TEXCOORD) : SV_Target
{
    return color * float4(uv, 0.0, 1.0);
}
