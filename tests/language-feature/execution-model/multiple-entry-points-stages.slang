// Test multiple entry points across vertex, fragment, and compute stages
// sharing a common utility function.

//TEST:SIMPLE(filecheck=CHECK_VERT): -target hlsl -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FRAG): -target hlsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_COMP): -target hlsl -entry compMain -stage compute

float brighten(float v) { return saturate(v * 2.0); }

struct VsOut
{
    float4 pos : SV_Position;
    float brightness : BRIGHTNESS;
};

// CHECK_VERT: SV_Position
// CHECK_VERT: BRIGHTNESS
[shader("vertex")]
VsOut vertMain(uint vid : SV_VertexID)
{
    VsOut o;
    o.pos = float4(float(vid), 0.0, 0.0, 1.0);
    o.brightness = brighten(float(vid) / 3.0);
    return o;
}

// CHECK_FRAG: SV_TARGET
[shader("fragment")]
float4 fragMain(float brightness : BRIGHTNESS) : SV_Target
{
    return float4(brightness, brightness, brightness, 1.0);
}

RWStructuredBuffer<float> buffer;

// CHECK_COMP: numthreads(64
[shader("compute")]
[numthreads(64,1,1)]
void compMain(uint3 tid : SV_DispatchThreadID)
{
    buffer[tid.x] = brighten(float(tid.x) / 64.0);
}
