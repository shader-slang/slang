//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main

// CHECK: OpEntryPoint

// Test compute semantics work correctly

[shader("compute")]
[numthreads(8, 8, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint groupIndex : SV_GroupIndex)
{
    // Compute operations
}


