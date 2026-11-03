// A whole thread-group-shared array passed to a `__ref_readonly` parameter of a differentiable
// function is read in place: directly, after forwarding the reference to a differentiable
// callee, and after forwarding it under `no_diff` to a non-differentiable one. The function that
// reads the reference waits at a barrier at which every invocation has written its own element,
// then reads its neighbor's, so invocations 0 to 3 should read 101, 102, 103 and 100 in each mode.
// A copy taken before the call would give the values from before the barrier, 1, 2, 3 and 0.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

void writeOwn(uint i)
{
    g[i] = 100.0 + float(i);
}

float readAfterNeighborWrites(__ref_readonly float a[4], uint i)
{
    GroupMemoryBarrierWithGroupSync();
    writeOwn(i);
    GroupMemoryBarrierWithGroupSync();
    return a[(i + 1) % 4];
}

[Differentiable]
float readDifferentiable(no_diff __ref_readonly float a[4], uint i, float x)
{
    no_diff(GroupMemoryBarrierWithGroupSync());
    no_diff(writeOwn(i));
    no_diff(GroupMemoryBarrierWithGroupSync());
    return a[(i + 1) % 4] * x;
}

[Differentiable]
float forwardToDifferentiable(no_diff __ref_readonly float a[4], uint i, float x)
{
    return readDifferentiable(a, i, x);
}

[Differentiable]
float forwardToNonDifferentiable(no_diff __ref_readonly float a[4], uint i, float x)
{
    return no_diff(readAfterNeighborWrites(a, i)) * x;
}

[Differentiable]
float direct(uint i, float x)
{
    return readDifferentiable(g, i, x);
}

[Differentiable]
float throughDifferentiable(uint i, float x)
{
    return forwardToDifferentiable(g, i, x);
}

[Differentiable]
float throughNonDifferentiable(uint i, float x)
{
    return forwardToNonDifferentiable(g, i, x);
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
    bwd_diff(direct)(i, x, 1.0);
    outputBuffer[i] = x.d;
    reset(i);
    outputBuffer[4 + i] = fwd_diff(direct)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(throughDifferentiable)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;
    reset(i);
    outputBuffer[12 + i] = fwd_diff(throughDifferentiable)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(throughNonDifferentiable)(i, x, 1.0);
    outputBuffer[16 + i] = x.d;
    reset(i);
    outputBuffer[20 + i] = fwd_diff(throughNonDifferentiable)(i, diffPair(1.0, 1.0)).d;
}

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
