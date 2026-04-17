// Test multiple entry points with different pipeline stages in a single file.
// Verifies correct compilation and emission when vertex, fragment, and compute
// entry points coexist. A shared utility function is used across stages.

//TEST:SIMPLE(filecheck=CHECK_VERT): -target hlsl -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FRAG): -target hlsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_COMP): -target hlsl -entry compMain -stage compute

// Shared utility used by multiple stages
float brighten(float v)
{
    return saturate(v * 2.0);
}

// --- Vertex entry point ---
struct VsOut
{
    float4 pos : SV_Position;
    float brightness : BRIGHTNESS;
};

// CHECK_VERT: SV_Position
// CHECK_VERT: BRIGHTNESS
[shader("vertex")]
VsOut vertMain(float3 position : POSITION, float lum : LUMINANCE)
{
    VsOut o;
    o.pos = float4(position, 1.0);
    o.brightness = brighten(lum);
    return o;
}

// --- Fragment entry point ---
// CHECK_FRAG: SV_TARGET
[shader("fragment")]
float4 fragMain(float brightness : BRIGHTNESS) : SV_Target
{
    float b = brighten(brightness);
    return float4(b, b, b, 1.0);
}

// --- Compute entry point ---
RWStructuredBuffer<float> outputBuf;

// CHECK_COMP: numthreads(64, 1, 1)
[shader("compute")]
[numthreads(64, 1, 1)]
void compMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuf[tid.x] = brighten(float(tid.x) / 64.0);
}
