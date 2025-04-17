//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type

//TEST_INPUT: set inputBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12], stride=4);
uniform int32_t* inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

// CHECK: type: int32_t
// CHECK-NEXT: 9
// CHECK-NEXT: 10
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let a = CoopVec<int32_t, 4>.load(inputBuffer, 0);
    let b = coopVecLoad<4>(inputBuffer + 4, 4*4);

    b.store(outputBuffer, 0);
    a.store(outputBuffer, 4*4);
}

