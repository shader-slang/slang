//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// WGSL's uniformity analysis treats every read of a `var<workgroup>` as possibly non-uniform,
// even when one thread wrote the value before a barrier. When such a read decides whether, or
// how many times, a barrier runs, Tint rejects the module unless the read goes through
// `workgroupUniformLoad`. The WGSL backend rewrites exactly those groupshared reads, and leaves
// a groupshared read alone when it does not control a barrier. The `-wgpu` row runs the
// emitted WGSL through Tint, which validates every function below.

//TEST_INPUT:ubuffer(data=[3], stride=4):name=counts
RWStructuredBuffer<uint> counts;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

groupshared uint gCount;
groupshared uint gVals[4];

// The loop bound decides how many times the barriers in the body run.
// CHECK-LABEL: fn loopBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint loopBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        gVals[t] = i + t;
        GroupMemoryBarrierWithGroupSync();
        acc += gVals[(t + 1) % 4];
        GroupMemoryBarrierWithGroupSync();
    }
    return acc;
}

// Copying the bound into a local first does not change which value controls the barrier.
// CHECK-LABEL: fn copyToLocal
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint copyToLocal(uint t)
{
    uint n = gCount;
    uint acc = 0;
    for (uint i = 0; i < n; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

void syncThreads()
{
    GroupMemoryBarrierWithGroupSync();
}

// A barrier inside a called function counts as a barrier in the loop body.
// CHECK-LABEL: fn helperBarrier
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint helperBarrier(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        syncThreads();
        acc += 2;
    }
    return acc;
}

// The callee's barrier loop is bounded by its parameter, so the caller's argument must be
// uniform.
// CHECK-LABEL: fn syncTimes
// CHECK-NOT: workgroupUniformLoad
uint syncTimes(uint n)
{
    uint acc = 0;
    for (uint i = 0; i < n; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 3;
    }
    return acc;
}

// CHECK-LABEL: fn paramBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint paramBound(uint t)
{
    return syncTimes(gCount);
}

// An early return skips the barrier that follows it.
// CHECK-LABEL: fn earlyReturn
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint earlyReturn(uint t)
{
    if (gCount == 0)
        return 0;
    GroupMemoryBarrierWithGroupSync();
    return 4;
}

// A `break` decides how many more times the barrier at the top of the loop runs.
// CHECK-LABEL: fn breakLoop
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint breakLoop(uint t)
{
    uint i = 0;
    while (true)
    {
        GroupMemoryBarrierWithGroupSync();
        if (i >= gCount)
            break;
        i++;
    }
    return i;
}

// The barrier runs after the branch on every path, so the branch needs no uniform read.
// CHECK-LABEL: fn barrierAfterBranch
// CHECK-NOT: workgroupUniformLoad
uint barrierAfterBranch(uint t)
{
    uint r = 0;
    if (t < gCount)
        r = 5;
    GroupMemoryBarrierWithGroupSync();
    return r;
}

// The groupshared read sits under a thread-dependent branch and controls no barrier. Reading
// it with `workgroupUniformLoad` there would put a barrier in non-uniform control flow.
// CHECK-LABEL: fn readUnderThreadBranch
// CHECK-NOT: workgroupUniformLoad
uint readUnderThreadBranch(uint t)
{
    uint r = 0;
    if (t == 0)
    {
        if (gCount > 2)
            r = 6;
    }
    GroupMemoryBarrierWithGroupSync();
    return r;
}

// A loop without a barrier needs no uniform bound.
// CHECK-LABEL: fn loopWithoutBarrier
// CHECK-NOT: workgroupUniformLoad
uint loopWithoutBarrier(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
        acc += i;
    return acc;
}

// CHECK-LABEL: fn computeMain
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    if (t == 0)
        gCount = counts[0];
    GroupMemoryBarrierWithGroupSync();

    uint base = t * 9;
    outputBuffer[base + 0] = loopBound(t);
    outputBuffer[base + 1] = copyToLocal(t);
    outputBuffer[base + 2] = helperBarrier(t);
    outputBuffer[base + 3] = paramBound(t);
    outputBuffer[base + 4] = earlyReturn(t);
    outputBuffer[base + 5] = breakLoop(t);
    outputBuffer[base + 6] = barrierAfterBranch(t);
    outputBuffer[base + 7] = readUnderThreadBranch(t);
    outputBuffer[base + 8] = loopWithoutBarrier(t);
}

// gCount is 3. loopBound sums gVals[(t + 1) % 4] = i + (t + 1) % 4 over i = 0, 1, 2.
// BUF: type: uint32_t
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 6
// BUF-NEXT: 9
// BUF-NEXT: 4
// BUF-NEXT: 3
// BUF-NEXT: 5
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 9
// BUF-NEXT: 3
// BUF-NEXT: 6
// BUF-NEXT: 9
// BUF-NEXT: 4
// BUF-NEXT: 3
// BUF-NEXT: 5
// BUF-NEXT: 0
// BUF-NEXT: 3
// BUF-NEXT: 12
// BUF-NEXT: 3
// BUF-NEXT: 6
// BUF-NEXT: 9
// BUF-NEXT: 4
// BUF-NEXT: 3
// BUF-NEXT: 5
// BUF-NEXT: 0
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 6
// BUF-NEXT: 9
// BUF-NEXT: 4
// BUF-NEXT: 3
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 3
