//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xslang... -Xdxc -Vd -X.

//DISABLE_TEST(compute):SIMPLE(filecheck=):-target dxil -profile cs_6_9 -entry computeMain
//DXIL: result code = 0

// CHECK: type: int32_t
// CHECK-NEXT: 35
// CHECK-NEXT: 76
// CHECK-NEXT: 117
// CHECK-NEXT: 158

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

// int8_t values [1 2 3 4] packed as uint8_t4_packed
//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=1),name=input
ByteAddressBuffer input;

// int8_t 4x4 row-major matrix [1..16] packed as uint8_t4_packed (4 values per element)
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16], stride=1),name=matrix
ByteAddressBuffer matrix;

//TEST_INPUT:ubuffer(data=[5 6 7 8], stride=4),name=bias
ByteAddressBuffer bias;

[numthreads(1, 1, 1)]
void computeMain()
{
    let vec = coopVecLoad<1, uint8_t4_packed>(input);
    let result = coopVecMatMulAddPacked<int32_t, 4>(
        vec,
        CoopVecComponentType::SignedInt8Packed,
        4,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
