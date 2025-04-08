//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -dx12-experimental -use-dxil -output-using-type -profile cs_6_9
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let result = CoopVec<int32_t, 4>(1,2,3,4);
    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];;
}
