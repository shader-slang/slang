// Test that [shader("...")] attribute correctly identifies various pipeline stages.
// Verifies HLSL emission includes correct stage-specific decorations/signatures.
// From the docs: pipeline types include rasterization, compute, and ray-tracing.

//TEST:SIMPLE(filecheck=CHECK_CS): -target hlsl -entry csMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_VS): -target hlsl -entry vsMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FS): -target hlsl -entry fsMain -stage fragment

// --- Compute shader ---
RWStructuredBuffer<float> buf;

// CHECK_CS: numthreads(8, 8, 1)
[shader("compute")]
[numthreads(8, 8, 1)]
void csMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x + tid.y * 8] = 1.0;
}

// --- Vertex shader ---
struct VsOut
{
    float4 pos : SV_Position;
};

// CHECK_VS: SV_Position
// CHECK_VS: POSITION
[shader("vertex")]
VsOut vsMain(float3 pos : POSITION)
{
    VsOut o;
    o.pos = float4(pos, 1.0);
    return o;
}

// --- Fragment shader ---
// CHECK_FS: SV_TARGET
[shader("fragment")]
float4 fsMain(float4 pos : SV_Position) : SV_Target
{
    return float4(pos.xy / 1024.0, 0, 1);
}
