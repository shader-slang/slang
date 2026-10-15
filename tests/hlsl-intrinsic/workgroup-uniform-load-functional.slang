// `workgroupUniformLoad` returns every thread the value one thread wrote before
// the call, and a write after the call does not change what other threads get
// back (shader-slang/slang#13631).
//
// Thread 0 writes 7, every thread loads it, then thread 0 overwrites it with 0.
// Without the first barrier other threads can read the old value 0; without
// the second they can read the overwrite. Every thread must report 7.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<uint> outputBuffer;

groupshared uint sharedVal;

[numthreads(64, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    if (gi == 0)
        sharedVal = 0;
    GroupMemoryBarrierWithGroupSync();

    if (gi == 0)
        sharedVal = 7;
    const uint n = workgroupUniformLoad(sharedVal);
    if (gi == 0)
        sharedVal = 0;

    if (gi % 8 == 7)
        outputBuffer[gi / 8] = n;
}

// CHECK: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
