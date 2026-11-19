//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type -compute-dispatch 2,1,1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -compute-dispatch 2,1,1

// When `SV_GroupID` is the only workgroup-uniform built-in a compute shader reads, its `vec3`
// value is passed by value to the functions that read it, and a workgroup barrier in a loop
// bounded by one of its components stays in uniform control flow (shader-slang/slang#8774). Two
// workgroups run, so the trip count and the output slot depend on the value passed in.

// WGSL: fn barrierLoop{{[_0-9]*}}({{.*}}[[P:[A-Za-z0-9_]+]] : vec3<u32>) -> u32
// WGSL: workgroupBarrier()
// WGSL: fn computeMain(
// WGSL-SAME: @builtin(workgroup_id) [[GID:[A-Za-z0-9_]+]] : vec3<u32>
// WGSL: barrierLoop{{[_0-9]*}}({{.*}}[[GID]])

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

in uint3 gGroupId : SV_GroupID;

groupshared uint gShared;

[noinline]
uint barrierLoop(uint groupIndex)
{
    uint total = 0;
    for (uint i = 0; i < gGroupId.x + 2; ++i)
    {
        if (groupIndex == 0)
            gShared = i;
        GroupMemoryBarrierWithGroupSync();
        total += gShared;
        GroupMemoryBarrierWithGroupSync();
    }
    return total;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint groupIndex: SV_GroupIndex)
{
    let total = barrierLoop(groupIndex);
    if (groupIndex == 0)
        outputBuffer[gGroupId.x] = total;
}

// BUF: 1
// BUF-NEXT: 3
