//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

RWStructuredBuffer<uint> resultBuffer;

uint3 getDispatchThreadID()
{
    return __builtinDispatchThreadID;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    resultBuffer[0] = getDispatchThreadID().x + tid.x;
}

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(
// METAL-SAME: uint3 tid_{{[0-9]+}} {{\[\[}}thread_position_in_grid{{\]\]}}
// METAL-NOT: thread_position_in_grid
// METAL-SAME: uint device* resultBuffer_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}}
