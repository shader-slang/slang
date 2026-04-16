// Test that WaveSize attribute is correctly emitted for HLSL target.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain

// CHECK_HLSL: [WaveSize(16)]

[WaveSize(16)]
[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
}
