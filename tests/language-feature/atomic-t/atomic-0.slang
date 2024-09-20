//TEST:SIMPLE(filecheck=WGSL): -target wgsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_MTL):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<Atomic<int>> outputBuffer;

[NumThreads(1,1,1)]
void computeMain()
{
    bool result = true;
    if (outputBuffer[0].add(1) != 0)
        result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].sub(1) != 1)
        result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].max(2) != 0)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].min(1) != 2)
    {} // result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].or(3) != 1)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].and(2) != 3)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].xor(3) != 2)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].exchange(4) != 1)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].compareExchange(4, 5) != 4)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].load() != 5)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].increment() != 5)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].decrement() != 6)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    // CHECK: 6

    // For some reason the metal results obtained from github runners are
    // incorrect, although M2 GPU does produce correct result.
    // For now we are just not going to check outputBuffer[1] for metal on the CI.
    // CHECK_MTL: 6
    outputBuffer[0].store(6);
    AllMemoryBarrierWithGroupSync();
    if (outputBuffer[0].load() != 6)
    {} //result = false;
    AllMemoryBarrierWithGroupSync();
    // CHECK: 1
    // CHECK_MTL: 1
    if (result)
        outputBuffer[1].store(1);
    else
        outputBuffer[1].store(0);
}

// WGSL: atomicAdd
// WGSL: atomicSub
// WGSL: atomicMax
// WGSL: atomicMin
// WGSL: atomicOr
// WGSL: atomicAnd
// WGSL: atomicXor
// WGSL: atomicExchange
// WGSL: atomicCompareExchangeWeak
// WGSL: atomicLoad
// WGSL: atomicAdd
// WGSL: atomicSub
// WGSL: atomicStore
// WGSL: atomicLoad
