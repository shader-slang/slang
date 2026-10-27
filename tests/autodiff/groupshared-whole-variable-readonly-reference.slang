// A whole thread-group-shared variable passed to a `__ref_readonly` parameter from differentiated
// code keeps its address: address elimination only rewrites addresses computed inside the
// function being differentiated, and a module-level `groupshared` variable is not one. Each
// invocation reads its neighbor's element after a barrier at which every invocation has written
// its own, so invocations 0 to 3 should see 101, 102, 103 and 100 in each mode. The
// groupshared-autodiff diagnostic tests rely on this to keep such an argument silent.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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
float throughNonDifferentiable(uint i, float x)
{
    return no_diff(readAfterNeighborWrites(g, i)) * x;
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
float throughDifferentiable(uint i, float x)
{
    return readDifferentiable(g, i, x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    var x = diffPair(1.0);
    bwd_diff(throughNonDifferentiable)(i, x, 1.0);
    outputBuffer[i] = x.d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[4 + i] = fwd_diff(throughNonDifferentiable)(i, diffPair(1.0, 1.0)).d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    x = diffPair(1.0);
    bwd_diff(throughDifferentiable)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;

    g[i] = float(i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[12 + i] = fwd_diff(throughDifferentiable)(i, diffPair(1.0, 1.0)).d;
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
