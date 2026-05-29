//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

RWStructuredBuffer<uint> resultBuffer;

uint3 getDispatchThreadID()
{
    return __builtinDispatchThreadID;
}

[shader("compute")]
[numthreads(4, 8, 2)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    resultBuffer[0] = getDispatchThreadID().x + tid;
}

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(
// METAL-SAME: uint3 sv_groupthreadid_{{[0-9]+}} {{\[\[}}thread_position_in_threadgroup{{\]\]}}
// METAL-SAME: uint3 sv_groupid_{{[0-9]+}} {{\[\[}}threadgroup_position_in_grid{{\]\]}}
// METAL-NOT: thread_position_in_grid
// METAL-SAME: uint device* resultBuffer_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}}
// METAL: uint3 sv_dispatchthreadid_{{[0-9]+}} = sv_groupid_{{[0-9]+}} * uint3(4U, 8U, 2U) + sv_groupthreadid_{{[0-9]+}};
