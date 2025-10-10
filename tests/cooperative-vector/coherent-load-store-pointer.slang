//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv

// coherent CoopVec operations crash the Nvidia driver.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -emit-spirv-directly

// Ensure SPIRV emits coherent operations here
// SPIRV: MakePointerVisible
// SPIRV: MakePointerAvailable

//TEST_INPUT: set inputBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12], stride=4);
uniform int32_t* inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4);
uniform int32_t* outputBuffer;

// CHECK: 9
// CHECK-NEXT: A
// CHECK-NEXT: B
// CHECK-NEXT: C
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    //// First half of input.
    let a = coopVecLoadCoherent<4, int32_t>(inputBuffer, 0, MemoryScope::Device);
    //// Second half of input.
    let b = coopVecLoadCoherent<4, int32_t>(inputBuffer + 4, 4 * 4, MemoryScope::Device);
    //// Store second half of input to first half of output buffer.
    b.storeCoherent(outputBuffer, 0, MemoryScope::Device);
    //// Store first half of input to second half of output buffer.
    a.storeCoherent(outputBuffer, 4 * 4, MemoryScope::Device);
}

