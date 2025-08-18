//TEST:SIMPLE(filecheck=SPIRV): -target spirv -emit-spirv-directly -profile cs_6_2 -entry computeMain

// Test for issue #8212: slangc crashes when initializing "static const" variable to cbuffer member for SPIRV target
// This test verifies that the compilation doesn't crash, which was the main issue.

extern static const float VALUE = Value;

cbuffer Options
{
    float Value;
}

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 thread_id : SV_DispatchThreadID)
{
    outputBuffer[thread_id.x] = float4(thread_id.y + VALUE);
}

// SPIRV: OpTypeFloat