//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// More shapes in which a groupshared read decides whether, or how many times, a barrier runs, and
// one in which it does not. Each of these must reach WGSL as a valid module; the `-wgpu` row runs
// the emitted WGSL through Tint.

//TEST_INPUT:ubuffer(data=[3 2 1], stride=4):name=counts
RWStructuredBuffer<uint> counts;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

groupshared uint gCount;
groupshared uint gIndex;
groupshared uint gFlag;
groupshared uint gArray[4];
groupshared uint4 gVector;
groupshared Atomic<uint> gAtomic;

// WGSL cannot take the address of a vector component, so the whole vector is read.
// CHECK-LABEL: fn vectorElement
// CHECK: : vec4<u32> = workgroupUniformLoad(&({{\(*}}gVector
uint vectorElement(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gVector[gIndex]; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// WGSL makes every later iteration depend on a condition that can `continue`, so the barrier at
// the top of the loop depends on `gFlag`.
// CHECK-LABEL: fn continueAfterBarrier
// CHECK: workgroupUniformLoad(&({{\(*}}gFlag
uint continueAfterBarrier(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < 4; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        if (gFlag == 0)
            continue;
        acc += 1;
    }
    return acc;
}

uint identity(uint v)
{
    return v;
}

// A value computed inside a loop depends on the loop's trip count.
// CHECK-LABEL: fn valueFromLoop
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint valueFromLoop(uint t)
{
    uint i = 0;
    uint x;
    do
    {
        x = identity(1);
        i++;
    } while (i < gCount);
    for (uint j = 0; j < x; ++j)
        GroupMemoryBarrierWithGroupSync();
    return x * 10 + i;
}

// CHECK-LABEL: fn atomicBound
// CHECK: workgroupUniformLoad(&({{\(*}}gAtomic
uint atomicBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gAtomic.load(); ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn allMemoryBarrierBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint allMemoryBarrierBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        AllMemoryBarrier();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn allMemoryBarrierWithGroupSyncBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint allMemoryBarrierWithGroupSyncBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        AllMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn deviceMemoryBarrierBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint deviceMemoryBarrierBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        DeviceMemoryBarrier();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn deviceMemoryBarrierWithGroupSyncBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint deviceMemoryBarrierWithGroupSyncBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        DeviceMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn groupMemoryBarrierBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint groupMemoryBarrierBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gCount; ++i)
    {
        GroupMemoryBarrier();
        acc += 1;
    }
    return acc;
}

// WGSL only allows `workgroupUniformLoad` itself in uniform control flow.
// CHECK-LABEL: fn explicitUniformLoad
// CHECK: workgroupUniformLoad(&({{\(*}}gFlag
uint explicitUniformLoad(uint t)
{
    uint r = 0;
    if (gFlag != 0)
        r = workgroupUniformLoad(gIndex);
    return r;
}

// CHECK-LABEL: fn switchOnCount
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint switchOnCount(uint t)
{
    uint r;
    switch (gCount)
    {
    case 3:
        GroupMemoryBarrierWithGroupSync();
        r = 7;
        break;
    default:
        r = 8;
        break;
    }
    return r;
}

// The index of a uniform load must itself be uniform.
// CHECK-LABEL: fn indexedBound
// CHECK-DAG: workgroupUniformLoad(&({{\(*}}gIndex
// CHECK-DAG: workgroupUniformLoad(&({{\(*}}gArray
uint indexedBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < gArray[gIndex]; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn minBound
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint minBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < min(gCount, 2u); ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// The inner loop absorbs its `continue`, so the barrier after it runs uniformly and the
// thread-dependent read of `gFlag` stays a plain read.
// CHECK-LABEL: fn continueInInnerLoop
// CHECK-NOT: workgroupUniformLoad
uint continueInInnerLoop(uint t)
{
    uint acc = 0;
    for (uint o = 0; o < 2; ++o)
    {
        for (uint i = 0; i < 4; ++i)
        {
            if (t == i && gFlag != 0)
                continue;
            acc += 1;
        }
        GroupMemoryBarrierWithGroupSync();
    }
    return acc;
}

// The caller needs the helper's result to be uniform, so the helper reads with
// `workgroupUniformLoad`.
// CHECK-LABEL: fn readCount
// CHECK: workgroupUniformLoad(&({{\(*}}gCount
uint readCount()
{
    return gCount;
}

// CHECK-LABEL: fn helperReturnBound
uint helperReturnBound(uint t)
{
    uint acc = 0;
    for (uint i = 0; i < readCount(); ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        acc += 1;
    }
    return acc;
}

// CHECK-LABEL: fn computeMain
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    if (t == 0)
    {
        gCount = counts[0];
        gIndex = counts[1];
        gFlag = counts[2];
        for (uint k = 0; k < 4; ++k)
            gArray[k] = k + 1;
        gVector = uint4(1, 2, 3, 4);
        gAtomic.store(3);
    }
    GroupMemoryBarrierWithGroupSync();

    uint base = t * 15;
    outputBuffer[base + 0] = vectorElement(t);
    outputBuffer[base + 1] = continueAfterBarrier(t);
    outputBuffer[base + 2] = valueFromLoop(t);
    outputBuffer[base + 3] = atomicBound(t);
    outputBuffer[base + 4] = allMemoryBarrierBound(t);
    outputBuffer[base + 5] = allMemoryBarrierWithGroupSyncBound(t);
    outputBuffer[base + 6] = deviceMemoryBarrierBound(t);
    outputBuffer[base + 7] = deviceMemoryBarrierWithGroupSyncBound(t);
    outputBuffer[base + 8] = groupMemoryBarrierBound(t);
    outputBuffer[base + 9] = explicitUniformLoad(t);
    outputBuffer[base + 10] = switchOnCount(t);
    outputBuffer[base + 11] = indexedBound(t);
    outputBuffer[base + 12] = minBound(t);
    outputBuffer[base + 13] = continueInInnerLoop(t);
    outputBuffer[base + 14] = helperReturnBound(t);
}

// BUF: type: uint32_t
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 13
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 7
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 13
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 7
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 13
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 7
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 13
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 7
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 6
// BUF-NEXT: 3
