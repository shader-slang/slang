// Test that [shader()] attributes correctly select entry points for different stages.

//TEST:SIMPLE(filecheck=CHECK_CS): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_VS): -target hlsl -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_FS): -target hlsl -entry fragMain -stage fragment

RWStructuredBuffer<float> outputBuffer;

// CHECK_CS: numthreads(64
[shader("compute")]
[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = float(tid.x);
}

struct VertexOut
{
    float4 pos : SV_Position;
};

// CHECK_VS: SV_Position
[shader("vertex")]
VertexOut vertMain(uint vid : SV_VertexID)
{
    VertexOut o;
    o.pos = float4(float(vid), 0.0, 0.0, 1.0);
    return o;
}

// CHECK_FS: SV_TARGET
[shader("fragment")]
float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    return float4(pos.x, pos.y, 0.0, 1.0);
}
