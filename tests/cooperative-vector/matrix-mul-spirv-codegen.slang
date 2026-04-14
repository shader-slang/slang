//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

// CHECK: OpCooperativeVectorMatrixMulNV {{.*}} MatrixBSignedComponentsKHR|MatrixResultSignedComponentsKHR
// CHECK: OpCooperativeVectorMatrixMulAddNV {{.*}} MatrixBSignedComponentsKHR|MatrixResultSignedComponentsKHR

RWStructuredBuffer<int32_t> outputBuffer;
ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<int8_t, 4> vec = coopVecLoad<4, int8_t>(input);

    let resultMatMul = coopVecMatMul<int32_t, 4, 4>(
        vec,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultMatMulAdd = coopVecMatMulAdd<int32_t, 4, 4>(
        vec,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    outputBuffer[0] = resultMatMul[0] + resultMatMulAdd[0];
}
