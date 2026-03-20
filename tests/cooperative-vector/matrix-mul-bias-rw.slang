//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Disabled because HLSL can't multiply from *RW*ByteAddressBuffers
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xslang... -Xdxc -Vd -X.
//DISABLE_DISABLE_TEST(compute):SIMPLE(filecheck=):-target dxil -profile cs_6_9 -entry computeMain
//DXIL: result code = 0

// CHECK: type: half
// CHECK-NEXT: 35.000000
// CHECK-NEXT: 76.000000
// CHECK-NEXT: 117.000000
// CHECK-NEXT: 158.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h], stride=2),name=input
ByteAddressBuffer input;

// float16_t 4x4 row-major matrix [1..16]
//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h 9.0h 10.0h 11.0h 12.0h 13.0h 14.0h 15.0h 16.0h], stride=2),name=matrix
RWByteAddressBuffer matrix;

//TEST_INPUT:ubuffer(data=[5.0h 6.0h 7.0h 8.0h], stride=2),name=bias
RWByteAddressBuffer bias;

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<float16_t, 4> vec = coopVecLoad<4, float16_t>(input);
    CoopVec<float16_t, 4> result = coopVecMatMulAdd<float16_t, 4, 4>(
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
