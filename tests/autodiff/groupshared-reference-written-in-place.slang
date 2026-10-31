// Thread-group-shared memory passed by reference from differentiated code is written in place by
// the callee. Each invocation writes its own element between two barriers and then reads its
// neighbor's, so invocations 0 to 3 should see 101, 102, 103 and 100 in each mode. We pass the
// memory three ways: an element to a writable `__ref` of a non-differentiable function under
// `no_diff`, a whole array to a writable `__ref` of a differentiable function, and a whole array to
// a `groupshared` parameter of a differentiable function. Address elimination passes the
// reference through rather than a copy, so a copy stored back after the call cannot overwrite the
// other invocations' writes, and the callee's reads see them.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

float writeOwnReadNeighbor(__ref float a, uint i)
{
    GroupMemoryBarrierWithGroupSync();
    a = 100.0 + float(i);
    GroupMemoryBarrierWithGroupSync();
    return g[(i + 1) % 4];
}

[Differentiable]
float elementToWritable(uint i, float x)
{
    return no_diff(writeOwnReadNeighbor(g[i], i)) * x;
}

[Differentiable]
float writeThroughRef(no_diff __ref float a[4], uint i, float x)
{
    no_diff(GroupMemoryBarrierWithGroupSync());
    a[i] = 100.0 + float(i);
    no_diff(GroupMemoryBarrierWithGroupSync());
    return a[(i + 1) % 4] * x;
}

[Differentiable]
float wholeToWritable(uint i, float x)
{
    return writeThroughRef(g, i, x);
}

[Differentiable]
float writeThroughGroupShared(no_diff groupshared float a[4], uint i, float x)
{
    no_diff(GroupMemoryBarrierWithGroupSync());
    a[i] = 100.0 + float(i);
    no_diff(GroupMemoryBarrierWithGroupSync());
    return a[(i + 1) % 4] * x;
}

[Differentiable]
float groupSharedParam(uint i, float x)
{
    return writeThroughGroupShared(g, i, x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    var x = diffPair(1.0);
    bwd_diff(elementToWritable)(i, x, 1.0);
    outputBuffer[i] = x.d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[4 + i] = fwd_diff(elementToWritable)(i, diffPair(1.0, 1.0)).d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    x = diffPair(1.0);
    bwd_diff(wholeToWritable)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[12 + i] = fwd_diff(wholeToWritable)(i, diffPair(1.0, 1.0)).d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    x = diffPair(1.0);
    bwd_diff(groupSharedParam)(i, x, 1.0);
    outputBuffer[16 + i] = x.d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[20 + i] = fwd_diff(groupSharedParam)(i, diffPair(1.0, 1.0)).d;
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
