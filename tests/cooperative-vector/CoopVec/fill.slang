//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil -output-using-type -profile cs_6_8 -Xslang... -Xdxc -Vd -X.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 10
// CHECK-NEXT: 10
// CHECK-NEXT: 10
// CHECK-NEXT: 10

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var result : CoopVec<int32_t, 4>;
    result.fill(10);
    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];;
}
