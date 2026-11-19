// A barrier condition read through a pointer parameter (#14390). The pointee can come from a
// caller that forwards its own `inout` argument, or from a store in the callee itself. Slang
// reads the groupshared values that reach the condition through `workgroupUniformLoad`. A
// caller's value that the callee always overwrites before the barrier keeps a plain load.

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

// CHECK-DAG: workgroupUniformLoad(&(gM_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gN_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gP_{{[0-9]+}}))
// CHECK-DAG: workgroupUniformLoad(&(gQ_{{[0-9]+}}))

// PLAIN-NOT: workgroupUniformLoad(&(gO_

//TEST_INPUT:ubuffer(data=[2 3 4 1 5], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:cbuffer(data=[1]):name=params
cbuffer params
{
    uint selector;
}

groupshared uint gM;
groupshared uint gN;
groupshared uint gO;
groupshared uint gP;
groupshared uint gQ;

// The barrier loop reads its bound through an `inout` parameter.
uint syncTimes(inout uint n)
{
    uint count = 0;
    for (uint i = 0; i < n; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        count += 1;
    }
    n = 0;
    return count;
}

// Forwards its own `inout` argument to the barrier loop.
uint syncForward(inout uint n)
{
    return syncTimes(n);
}

// Overwrites the pointee before the barrier loop reads it, so the caller's value is not read.
uint resetAndSync(inout uint n)
{
    n = gN;
    uint count = 0;
    for (uint i = 0; i < n; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        count += 1;
    }
    return count;
}

// Overwrites the pointee only on one path, so the caller's value can still be read.
uint maybeResetAndSync(inout uint n)
{
    if (selector == 0)
        n = gP;
    uint count = 0;
    for (uint i = 0; i < n; ++i)
    {
        GroupMemoryBarrierWithGroupSync();
        count += 1;
    }
    return count;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    if (tid.x == 0)
    {
        gM = inputBuffer[0];
        gN = inputBuffer[1];
        gO = inputBuffer[2];
        gP = inputBuffer[3];
        gQ = inputBuffer[4];
    }
    GroupMemoryBarrierWithGroupSync();

    uint result = 0;

    uint forwarded = gM;
    result += syncForward(forwarded);

    uint overwritten = gO;
    result += 10 * resetAndSync(overwritten);

    uint maybe = gQ;
    result += 100 * maybeResetAndSync(maybe);

    outputBuffer[tid.x] = result;
}

// 2 + 10 * 3 + 100 * 5 = 532.
// BUF: 532
// BUF-NEXT: 532
// BUF-NEXT: 532
// BUF-NEXT: 532
