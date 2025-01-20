//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil -output-using-type -profile cs_6_8 -Xslang... -Xdxc -Vd -X.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let result = CoopVec<float, 4>(int(1),half(2),uint(3),double(4));
    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];;
}
