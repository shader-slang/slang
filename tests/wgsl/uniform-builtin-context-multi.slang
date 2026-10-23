//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOPRIV): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type -compute-dispatch 2,1,1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -compute-dispatch 2,1,1

// When a compute shader reads more than one workgroup-uniform built-in, the values are combined in
// one function-local context that holds only uniform values, and functions that read them receive
// a `ptr<function, ...>` to it. A workgroup barrier and a subgroup operation that depend on those
// values then stay in uniform control flow (shader-slang/slang#8774). `gGroupId` is a user-declared
// global input; it goes through the same shadow path as the core module's built-ins.

// WGSL: struct [[T:KernelContext[_0-9]*]]
// WGSL-NEXT: {
// WGSL-DAG: {{[A-Za-z0-9_]+}} : vec3<u32>,
// WGSL-DAG: {{[A-Za-z0-9_]+}} : u32,
// WGSL: fn barrierLoop{{[_0-9]*}}({{.*}} [[P:[A-Za-z0-9_]+]] : ptr<function, [[T]]>)
// WGSL: (*[[P]]).
// WGSL: workgroupBarrier()
// WGSL: fn subgroupLoop{{[_0-9]*}}({{ *}}[[Q:[A-Za-z0-9_]+]] : ptr<function, [[T]]>) -> u32
// WGSL: u32(256) / (*[[Q]]).
// WGSL: subgroupAdd(
// WGSL: fn computeMain(
// WGSL: var [[C:[A-Za-z0-9_]+]] : [[T]];
// WGSL: barrierLoop{{[_0-9]*}}({{.*}}&([[C]]))

// NOPRIV-NOT: var<private>

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

in uint3 gGroupId : SV_GroupID;

groupshared uint gShared;

static const uint kGroupSize = 256;

[noinline]
void barrierLoop(uint groupIndex)
{
    for (uint i = 0; i < gGroupId.x + 2; ++i)
    {
        if (groupIndex == 0)
            gShared = i;
        GroupMemoryBarrierWithGroupSync();
    }
}

[noinline]
uint subgroupLoop()
{
    uint total = 0;
    for (uint i = 0; i < kGroupSize / WaveGetLaneCount(); ++i)
        total += WaveActiveSum(1u);
    return total;
}

[shader("compute")]
[numthreads(kGroupSize, 1, 1)]
void computeMain(uint groupIndex: SV_GroupIndex)
{
    barrierLoop(groupIndex);
    let total = subgroupLoop();
    if (groupIndex == 0)
    {
        outputBuffer[gGroupId.x * 2] = gShared;
        outputBuffer[gGroupId.x * 2 + 1] = total;
    }
}

// Two workgroups run, so the barrier loop's trip count and the output slots depend on the
// `workgroup_id` value stored into the context; an uninitialized field would read 0 in both.
// BUF: 1
// BUF-NEXT: 256
// BUF-NEXT: 2
// BUF-NEXT: 256
