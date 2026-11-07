// Each invocation writes its own element of a thread-group-shared array inside a differentiable
// function, then reads its neighbor's element after a barrier. The derivative with respect to `x`
// is the neighbor's value, so invocations 0 to 3 should output 10, 20, 30 and 0.
//
// Disabled: address elimination, which prepares a function for differentiation, turns the
// element write into a read-modify-write of the whole array. Invocations then write back stale
// copies of each other's elements, and the neighbor reads -1.0, the value stored before the
// call.

//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[Differentiable]
float f(uint i, float x)
{
    g[i] = float(i) * 10.0;
    GroupMemoryBarrierWithGroupSync();
    return g[(i + 1) % 4] * x;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    g[i] = -1.0;
    GroupMemoryBarrierWithGroupSync();
    var x = diffPair(3.0);
    bwd_diff(f)(i, x, 1.0);
    outputBuffer[i] = x.d;
}

// CHECK: 10.0
// CHECK-NEXT: 20.0
// CHECK-NEXT: 30.0
// CHECK-NEXT: 0.0
