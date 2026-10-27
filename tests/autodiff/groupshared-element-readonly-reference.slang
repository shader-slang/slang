// Each invocation passes its neighbor's element of a thread-group-shared array to a
// `__ref_readonly` parameter from differentiated code, and the callee reads it after a barrier at
// which every invocation has written its own element. A reference reads the memory when it is
// accessed, so invocations 0 to 3 should read 101, 102, 103 and 100 in each mode.
//
// Disabled: address elimination, which prepares a function for differentiation, copies the
// element into a temporary before the call whatever the parameter's access, so the callee reads
// the values from before the call: 1, 2, 3 and 0. It also stores the copy back after the call,
// overwriting other invocations' writes, even though the parameter is read-only.

//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared uint g[4];

uint readAfterNeighborWrites(__ref_readonly uint a, uint i)
{
    GroupMemoryBarrierWithGroupSync();
    g[i] = 100 + i;
    GroupMemoryBarrierWithGroupSync();
    return a;
}

[Differentiable]
float caller(uint i, float x)
{
    uint v = no_diff(readAfterNeighborWrites(g[(i + 1) % 4], i));
    return x * float(v);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    g[i] = i;
    GroupMemoryBarrierWithGroupSync();
    var x = diffPair(1.0);
    bwd_diff(caller)(i, x, 1.0);
    outputBuffer[i] = x.d;

    g[i] = i;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[4 + i] = fwd_diff(caller)(i, diffPair(1.0, 1.0)).d;
}

// CHECK: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 103.0
// CHECK-NEXT: 100.0
