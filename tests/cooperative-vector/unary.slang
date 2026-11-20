//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-render-feature cooperative-vector -output-using-type -xslang -skip-spirv-validation
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK):-output-using-type

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain -DRWBAB
//DXIL: result code = 0

// CHECK: type: int32_t
// CHECK-NEXT: -1
// CHECK-NEXT: -2
// CHECK-NEXT: -3
// CHECK-NEXT: -4

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input1

#if defined(RWBAB)
RWByteAddressBuffer input1;
#else
ByteAddressBuffer input1;
#endif

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<int32_t, 4> vec1 = coopVecLoad<4, int32_t>(input1);
    CoopVec<int32_t, 4> negVec = -vec1;

    for(int i = 0; i < negVec.getCount(); ++i)
        outputBuffer[i] = negVec[i];
}
