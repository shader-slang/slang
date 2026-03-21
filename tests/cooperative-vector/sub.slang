//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain -DRWBAB
//DXIL: result code = 0

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

//TEST_INPUT:ubuffer(data=[2 3 4 5], stride=4),name=input1
//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input2

#if defined(RWBAB)
RWByteAddressBuffer input1;
RWByteAddressBuffer input2;
#else
ByteAddressBuffer input1;
ByteAddressBuffer input2;
#endif

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<int, 4> vec1 = coopVecLoad<4, int32_t>(input1);
    CoopVec<int, 4> vec2 = coopVecLoad<4, int32_t>(input2);

    CoopVec<int, 4> result = vec1 - vec2;

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
