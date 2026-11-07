// Thread-group-shared memory passed by reference from differentiated code, to a non-differentiable
// function called under `no_diff`, is accessed in place by the callee. Address elimination, which
// prepares a function for differentiation, passes the reference through rather than a copy.
//
// We pass an element and a whole array, each to a `__ref_readonly` and to a `__ref` parameter.
// Each invocation writes its own element between two barriers and then reads its neighbor's
// through the reference, so invocations 0 to 3 should read 101, 102, 103 and 100 in each mode. A
// copy taken before the call would give the values from before the barrier, 1, 2, 3 and 0, and
// storing the copy back after the call would overwrite the other invocations' writes.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -experimental-feature -line-directive-mode none

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

void writeOwn(uint i)
{
    g[i] = 100.0 + float(i);
}

float readElementAfterNeighborWrites(__ref_readonly float a, uint i)
{
    GroupMemoryBarrierWithGroupSync();
    writeOwn(i);
    GroupMemoryBarrierWithGroupSync();
    return a;
}

float readArrayAfterNeighborWrites(__ref_readonly float a[4], uint i)
{
    GroupMemoryBarrierWithGroupSync();
    writeOwn(i);
    GroupMemoryBarrierWithGroupSync();
    return a[(i + 1) % 4];
}

float writeElementThenReadNeighbor(__ref float a, uint i)
{
    GroupMemoryBarrierWithGroupSync();
    a = 100.0 + float(i);
    GroupMemoryBarrierWithGroupSync();
    return g[(i + 1) % 4];
}

float writeArrayThenReadNeighbor(__ref float a[4], uint i)
{
    GroupMemoryBarrierWithGroupSync();
    a[i] = 100.0 + float(i);
    GroupMemoryBarrierWithGroupSync();
    return a[(i + 1) % 4];
}

[Differentiable]
float readElement(uint i, float x)
{
    return no_diff(readElementAfterNeighborWrites(g[(i + 1) % 4], i)) * x;
}

[Differentiable]
float readArray(uint i, float x)
{
    return no_diff(readArrayAfterNeighborWrites(g, i)) * x;
}

[Differentiable]
float writeElement(uint i, float x)
{
    return no_diff(writeElementThenReadNeighbor(g[i], i)) * x;
}

[Differentiable]
float writeArray(uint i, float x)
{
    return no_diff(writeArrayThenReadNeighbor(g, i)) * x;
}

void reset(uint i)
{
    GroupMemoryBarrierWithGroupSync();
    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    reset(i);
    var x = diffPair(1.0);
    bwd_diff(readElement)(i, x, 1.0);
    outputBuffer[i] = x.d;
    reset(i);
    outputBuffer[4 + i] = fwd_diff(readElement)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(readArray)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;
    reset(i);
    outputBuffer[12 + i] = fwd_diff(readArray)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(writeElement)(i, x, 1.0);
    outputBuffer[16 + i] = x.d;
    reset(i);
    outputBuffer[20 + i] = fwd_diff(writeElement)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(writeArray)(i, x, 1.0);
    outputBuffer[24 + i] = x.d;
    reset(i);
    outputBuffer[28 + i] = fwd_diff(writeArray)(i, diffPair(1.0, 1.0)).d;
}

// The derivatives never read or write the array as a whole.
// HLSL-NOT: = g_0;
// HLSL-NOT: g_0 =
// HLSL: void computeMain
// HLSL-NOT: = g_0;
// HLSL-NOT: g_0 =

// CHECK: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
