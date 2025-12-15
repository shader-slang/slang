//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain

// Test that compute-only semantics are validated correctly

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID,  // Valid in compute stage
    uint3 groupID : SV_GroupID,  // Valid in compute stage
    uint3 groupThreadID : SV_GroupThreadID,  // Valid in compute stage
    uint groupIndex : SV_GroupIndex)  // Valid in compute stage
{
    // Do nothing
}

// CHECK: computeMain

