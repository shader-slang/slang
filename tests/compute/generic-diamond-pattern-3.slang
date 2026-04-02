//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// Two-module version of the diamond pattern test.
// Module 1 (generic-diamond-pattern-3-lib) defines the diamond interfaces,
// BottomImpl, and GenFoo.
// This module adds a new interface MidLeftExtra : MidLeft, extends BottomImpl
// to conform to it, and exercises the result.

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import generic_diamond_pattern_3_lib;

interface MidLeftExtra : Top
{
    float getExtra();
};

extension BottomImpl : MidLeftExtra
{
    float getExtra() { return x * 2.0; }
};

extension BottomImpl : Top {};

T test<T : MidRight>(GenFoo<T> val)
{
    return val.x;
}

float testExtra<T : MidLeftExtra>(T val)
{
    return val.getExtra();
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // Test that the internal representation of (BottomImpl : Top) with additional extensions
    // is still compatible with the one from the other module that sees a different heirarchy.
    //
    GenFoo<BottomImpl> outVal;
    outVal = getGenFoo(tid);

    BottomImpl bi = test(outVal); // id * 3
    float extra = testExtra(bi); // id * 3 * 2 = id * 6
    outputBuffer[tid] = bi.x + extra; // id * 3 + id * 6 = id * 9

    // CHECK: 0.0
    // CHECK-NEXT: 9.0
    // CHECK-NEXT: 18.0
    // CHECK-NEXT: 27.0
}
