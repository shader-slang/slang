// A groupshared value that decides whether a barrier runs can also reach the barrier condition
// through a partly written local array, or as the result of a call that is not side-effect free
// (#14390). Slang reads it through `workgroupUniformLoad` in those cases too. A call made in
// non-uniform control flow keeps a plain load for its own argument.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -O0
// The wgsl-spirv-asm rows run the WGSL through Tint, which rejects a barrier in
// non-uniform control flow; so does the -wgpu row at pipeline creation.
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry computeMain -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// TINT: OpControlBarrier

// CHECK-DAG: workgroupUniformLoad(&(gA_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gB_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gD_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gF_{{[0-9]+}}))
// CHECK-NOT: workgroupUniformLoad(&(gE_

//TEST_INPUT:ubuffer(data=[2 3 1 4 5], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):name=logBuffer
RWStructuredBuffer<uint> logBuffer;

//TEST_INPUT:cbuffer(data=[1]):name=params
cbuffer params
{
    uint selector;
}

groupshared uint gA;
groupshared uint gB;
groupshared uint gD;
groupshared uint gE;
groupshared uint gF;

// Reads groupshared memory itself, so the call is not side-effect free.
uint countPlusOne()
{
    return gD + 1;
}

// Has a side effect, so the call is not side-effect free either.
uint logAndDouble(uint value)
{
    logBuffer[0] = value;
    return value * 2;
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

    outputBuffer[tid.x] = result;
}

// 3 + 2 * 10 + 8 * 100 = 823, and thread 3 adds 2 * 5.
// BUF: 823
// BUF-NEXT: 823
// BUF-NEXT: 823
// BUF-NEXT: 833
