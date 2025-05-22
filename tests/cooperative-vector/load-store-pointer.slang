//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -emit-spirv-directly

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
    // First half of input.
    let a = CoopVec<int32_t, 4>.load(inputBuffer, 0);
    // Second half of input.
    let b = CoopVec<int32_t, 4>.load(inputBuffer + 4, 4*4);

    // Store second half of input to first half of output buffer.
    b.store(outputBuffer, 0);
    // Store first half of input to second half of output buffer.
    a.store(outputBuffer, 4*4);
}

