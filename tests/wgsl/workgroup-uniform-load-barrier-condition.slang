// WGSL's uniformity analysis treats a read of `var<workgroup>` memory as non-uniform unless it is a
// `workgroupUniformLoad`, so a barrier in a loop or branch that a groupshared value controls fails
// pipeline creation (#14390). HLSL already requires such a barrier to be reached by the whole
// group, so Slang reads the controlling groupshared value through `workgroupUniformLoad`. Reads
// that do not decide whether a barrier runs stay plain loads.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -O0
// The wgsl-spirv-asm rows run the WGSL through Tint, which rejects a barrier in
// non-uniform control flow; so does the -wgpu row at pipeline creation.
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// TINT: OpControlBarrier

// The reads of `gValues` in `exchange` and `computeMain` do not decide whether a barrier runs,
// so they stay plain loads.
//TEST:SIMPLE(filecheck=PLAIN): -target wgsl -stage compute -entry computeMain
// PLAIN-NOT: workgroupUniformLoad(&(gValues

//TEST_INPUT:ubuffer(data=[3 1 2 1 2 2 1 3 1 0], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Range
{
    uint start;
    uint end;
};

groupshared uint gCount;
groupshared uint gFlag;
groupshared uint gNoBarrierCount;
groupshared uint gNoBarrierFlag;
groupshared uint gCounts[2];
groupshared Range gRange;
groupshared uint gSelector;
groupshared uint gDone;
groupshared uint gValues[4];

void sync()
{
    GroupMemoryBarrierWithGroupSync();
}

// The bound is a parameter; the caller's argument is the groupshared read.
uint exchange(uint count, uint tid)
{
    uint sum = 0;
    for (uint i = 0; i < count; ++i)
    {
        gValues[tid] = i;
        GroupMemoryBarrierWithGroupSync();
        sum += gValues[(tid + 1) % 4];
        GroupMemoryBarrierWithGroupSync();
    }
    return sum;
}

// CHECK-LABEL: fn computeMain(
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    if (tid.x == 0)
    {
        gCount = inputBuffer[0];
        gFlag = inputBuffer[1];
        gNoBarrierCount = inputBuffer[2];
        gNoBarrierFlag = inputBuffer[3];
        gCounts[0] = inputBuffer[4];
        gCounts[1] = inputBuffer[5];
        gRange.start = inputBuffer[6];
        gRange.end = inputBuffer[7];
        gSelector = inputBuffer[8];
        gDone = inputBuffer[9];
    }
    GroupMemoryBarrierWithGroupSync();

    uint result = 0;

    // A `while` condition, with the barrier in a callee.
    // CHECK: workgroupUniformLoad(&(gCount_{{[0-9]+}}))
    uint i = 0;
    while (i < gCount)
    {
        sync();
        i++;
    }

    // A `do`-`while` condition.
    // CHECK: workgroupUniformLoad(&(gCounts_{{[0-9]+}}[i32(1)]))
    uint j = 0;
    do
    {
        GroupMemoryBarrierWithGroupSync();
        j++;
    } while (j < gCounts[1]);

    // An `if` that guards a barrier.
    // CHECK: workgroupUniformLoad(&(gFlag_{{[0-9]+}}))
    if (gFlag != 0)
    {
        GroupMemoryBarrierWithGroupSync();
        result += 1;
    }

    // A bound read once into a local before the loop.
    // CHECK: workgroupUniformLoad(&(gCounts_{{[0-9]+}}[i32(0)]))
    uint n = gCounts[0] * 2;
    for (uint k = 0; k < n; ++k)
        GroupMemoryBarrierWithGroupSync();

    // A loop counter that starts at a groupshared value, and a bound in a groupshared struct.
    // CHECK: workgroupUniformLoad(&(gRange_{{[0-9]+}}.start_{{[0-9]+}}))
    // CHECK: workgroupUniformLoad(&(gRange_{{[0-9]+}}.end_{{[0-9]+}}))
    for (uint k = gRange.start; k < gRange.end; ++k)
        GroupMemoryBarrierWithGroupSync();

    // A groupshared argument for a callee's loop bound.
    // CHECK: workgroupUniformLoad(&(gCount_{{[0-9]+}}))
    // CHECK: exchange_{{[0-9]+}}(
    result += exchange(gCount, tid.x);

    // A `switch` with a barrier in one case.
    // CHECK: workgroupUniformLoad(&(gSelector_{{[0-9]+}}))
    switch (gSelector)
    {
    case 1:
        GroupMemoryBarrierWithGroupSync();
        result += 10;
        break;
    default:
        break;
    }

    // Groupshared values that do not control a barrier stay plain loads.
    // CHECK-NOT: workgroupUniformLoad(&(gNoBarrier
    for (uint k = 0; k < gNoBarrierCount; ++k)
        result += k;
    if (gNoBarrierFlag != 0)
        result += 100;
    if (tid.x == 3)
        result += gValues[2];

    // An early return before a barrier.
    // CHECK: workgroupUniformLoad(&(gDone_{{[0-9]+}}))
    if (gDone != 0)
    {
        outputBuffer[tid.x] = result;
        return;
    }
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = result + 1000;
}

// The `exchange` loop leaves 2 in every `gValues` element, which only thread 3 adds.
// BUF: 1115
// BUF-NEXT: 1115
// BUF-NEXT: 1115
// BUF-NEXT: 1117
