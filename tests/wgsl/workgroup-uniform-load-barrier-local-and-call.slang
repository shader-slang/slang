// A groupshared value that decides whether a barrier runs can also reach the barrier condition
// through a local variable or array, or through a call that is not side-effect free: as its
// result, through an `out` or `inout` parameter, or through a nested call (#14390). Slang reads it through
// `workgroupUniformLoad` in those cases too. A call made in non-uniform control flow keeps a plain
// load for its own argument, and so does a store that cannot reach the barrier condition.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -O0
// The wgsl-spirv-asm rows run the WGSL through Tint, which rejects a barrier in
// non-uniform control flow; so does the -wgpu row at pipeline creation.
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain -O0
//TEST:SIMPLE(filecheck=PLAIN): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=PLAIN): -target wgsl -stage compute -entry computeMain -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// TINT: OpControlBarrier

// CHECK-DAG: workgroupUniformLoad(&(gA_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gB_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gD_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gF_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gC_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gFlag_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gG_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gI_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gK_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gS_{{[0-9]+}}))

// PLAIN-NOT: workgroupUniformLoad(&(gE_
// PLAIN-NOT: workgroupUniformLoad(&(gH_
// PLAIN-NOT: workgroupUniformLoad(&(gJ_

//TEST_INPUT:ubuffer(data=[2 3 1 4 5 2 1 3 7 2 9 2 1], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Every thread calls the logging helpers, so they write the log atomically.
//TEST_INPUT:ubuffer(data=[0], stride=4):name=logBuffer
RWStructuredBuffer<Atomic<uint>> logBuffer;

//TEST_INPUT:cbuffer(data=[1]):name=params
cbuffer params
{
    uint selector;
}

groupshared uint gA;
groupshared uint gB;
groupshared uint gC;
groupshared uint gD;
groupshared uint gE;
groupshared uint gF;
groupshared uint gFlag;
groupshared uint gG;
groupshared uint gH;
groupshared uint gI;
groupshared uint gJ;
groupshared uint gK;
groupshared uint gS;

// Reads groupshared memory itself, so the call is not side-effect free.
uint countPlusOne()
{
    return gD + 1;
}

// Has a side effect, so the call is not side-effect free either.
uint logAndDouble(uint value)
{
    logBuffer[0].add(value);
    return value * 2;
}

// The groupshared read is in a block after a branch, which runs on every call without being
// the entry block. Calling this under `if (gFlag != 0)` reaches its `workgroupUniformLoad`, so
// `gFlag` has to be uniform as well.
uint readCount(uint k)
{
    uint extra = 0;
    if (k > 1)
        extra = 1;
    return gC + extra;
}

// Writes its result through an `out` parameter.
void readBound(out uint n)
{
    logBuffer[0].add(1);
    n = gG;
}

// Returns the result of a nested call on its parameter.
uint tripleLogged(uint value)
{
    logBuffer[0].add(value + 1);
    return value * 3;
}

// The barrier loop reads its bound through an `inout` parameter.
void syncTimes(inout uint n)
{
    for (uint i = 0; i < n; ++i)
        GroupMemoryBarrierWithGroupSync();
    n = 0;
}

uint outerBound(uint value)
{
    logBuffer[0].add(value);
    return tripleLogged(value);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    if (tid.x == 0)
    {
        gA = inputBuffer[0];
        gB = inputBuffer[1];
        gD = inputBuffer[2];
        gF = inputBuffer[3];
        gE = inputBuffer[4];
        gC = inputBuffer[5];
        gFlag = inputBuffer[6];
        gG = inputBuffer[7];
        gH = inputBuffer[8];
        gI = inputBuffer[9];
        gJ = inputBuffer[10];
        gK = inputBuffer[11];
        gS = inputBuffer[12];
    }
    GroupMemoryBarrierWithGroupSync();

    uint result = 0;

    // A bound read from a local array that is written one element at a time.
    uint bounds[2];
    bounds[0] = gA;
    bounds[1] = gB;
    for (uint k = 0; k < bounds[selector]; ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 1;
    }

    // A bound returned by a call that reads groupshared memory.
    for (uint k = 0; k < countPlusOne(); ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 10;
    }

    // A bound returned by a call with a side effect, from a groupshared argument.
    for (uint k = 0; k < logAndDouble(gF); ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 100;
    }

    // The same callee in non-uniform control flow: its argument stays a plain load.
    if (tid.x == 3)
        result += logAndDouble(gE);

    // A groupshared read after a branch in the callee, and the same callee under a branch.
    for (uint k = 0; k < readCount(selector); ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 1000;
    }
    if (gFlag != 0)
        result += readCount(0);

    // A bound written through an `out` parameter.
    uint n;
    readBound(n);
    for (uint k = 0; k < n; ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 10000;
    }

    // A later store in non-uniform control flow cannot reach the bound, so it stays a plain load.
    uint later[2];
    later[0] = 1;
    later[1] = 2;
    for (uint k = 0; k < later[selector]; ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 100000;
    }
    if (tid.x == 3)
        later[0] = gH;
    result += later[0];

    // A bound from a nested call. The outer callee in non-uniform control flow keeps a plain load.
    for (uint k = 0; k < outerBound(gI); ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 1000000;
    }
    if (tid.x == 3)
        result += outerBound(gJ);

    // A bound read through the callee's `inout` parameter, from a local of the caller.
    uint syncCount = gK;
    syncTimes(syncCount);
    result += syncCount;

    // A bound written into two components of a local vector by a swizzled store.
    uint4 quad = uint4(0, 0, 0, 0);
    quad.yz = uint2(gS, gS);
    for (uint k = 0; k < quad[selector]; ++k)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 10000000;
    }

    outputBuffer[tid.x] = result;
}

// 3 + 2 * 10 + 8 * 100 + 2 * 1000 + 2 + 3 * 10000 + 2 * 100000 + 1 + 6 * 1000000 + 10000000
// = 16232826. Thread 3 also adds 2 * 5 and 3 * 9, and reads 7 instead of 1 from `later[0]`.
// BUF: 16232826
// BUF-NEXT: 16232826
// BUF-NEXT: 16232826
// BUF-NEXT: 16232869
