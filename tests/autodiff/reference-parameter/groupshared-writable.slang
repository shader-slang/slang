// A whole thread-group-shared array passed to a writable reference parameter of a differentiable
// function, either `__ref` or `groupshared`, is written in place. Each invocation adds 100 to its
// own element between two barriers and then reads its neighbor's, so invocations 0 to 3 should
// read 101, 102, 103 and 100 in each mode. A copy stored back after the call would overwrite the
// other invocations' writes, and a backward derivative that ran the write twice would read 201,
// 202, 203 and 200. We differentiate a caller of each function, a caller that calls the
// `groupshared` one under `no_diff`, and the `groupshared` one directly.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[Differentiable]
float incrementThroughRef(no_diff __ref float a[4], uint i, float x)
{
    no_diff(GroupMemoryBarrierWithGroupSync());
    a[i] = a[i] + 100.0;
    no_diff(GroupMemoryBarrierWithGroupSync());
    return a[(i + 1) % 4] * x;
}

[Differentiable]
float incrementThroughGroupShared(no_diff groupshared float a[4], uint i, float x)
{
    no_diff(GroupMemoryBarrierWithGroupSync());
    a[i] = a[i] + 100.0;
    no_diff(GroupMemoryBarrierWithGroupSync());
    return a[(i + 1) % 4] * x;
}

[Differentiable]
float throughRef(uint i, float x)
{
    return incrementThroughRef(g, i, x);
}

[Differentiable]
float throughGroupShared(uint i, float x)
{
    return incrementThroughGroupShared(g, i, x);
}

[Differentiable]
float underNoDiff(uint i, float x)
{
    return no_diff(incrementThroughGroupShared(g, i, 1.0)) * x;
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
    bwd_diff(throughRef)(i, x, 1.0);
    outputBuffer[i] = x.d;
    reset(i);
    outputBuffer[4 + i] = fwd_diff(throughRef)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(throughGroupShared)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;
    reset(i);
    outputBuffer[12 + i] = fwd_diff(throughGroupShared)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(underNoDiff)(i, x, 1.0);
    outputBuffer[16 + i] = x.d;
    reset(i);
    outputBuffer[20 + i] = fwd_diff(underNoDiff)(i, diffPair(1.0, 1.0)).d;

    reset(i);
    x = diffPair(1.0);
    bwd_diff(incrementThroughGroupShared)(g, i, x, 1.0);
    outputBuffer[24 + i] = x.d;
    reset(i);
    outputBuffer[28 + i] = fwd_diff(incrementThroughGroupShared)(g, i, diffPair(1.0, 1.0)).d;
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
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
