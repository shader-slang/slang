//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xslang... -Xdxc -Vd -X.

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//DXIL: result code = 0

// CHECK: type: half
// CHECK-NEXT: 36.000000
// CHECK-NEXT: 77.000000
// CHECK-NEXT: 118.000000
// CHECK-NEXT: 159.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h], stride=2),name=input
ByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h 9.0h 10.0h 11.0h 12.0h 13.0h 14.0h 15.0h 16.0h], stride=2),name=matrix
ByteAddressBuffer matrix;

//TEST_INPUT:ubuffer(data=[5.0h 6.0h 7.0h 8.0h], stride=2),name=bias
ByteAddressBuffer bias;

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<float16_t, 4> vec = coopVecLoad<4, float16_t>(input);
    var result = CoopVec<float16_t, 4>(1.0h);
    result.matMulAddAccum(
        vec,
        CoopVecComponentType::Float16,
        matrix,
        0,
        CoopVecComponentType::Float16,
        bias,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false,
        8
    );

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
