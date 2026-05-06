//TEST:SIMPLE(filecheck=CHECK609): -target hlsl -entry computeMain -profile cs_6_9
//TEST:SIMPLE(filecheck=CHECK610): -target hlsl -entry computeMain -profile cs_6_10

// CHECK609: __builtin_MatVecMul(__slang_r,
// CHECK609: __builtin_MatVecMulAdd(__slang_r,
// CHECK609: ByteAddressBuffer matrix_0 : register
// CHECK609: ByteAddressBuffer bias_0 : register
// CHECK609-COUNT-4: __slang_linalg_Mul

// CHECK610: dx::linalg::Multiply<
// CHECK610: dx::linalg::MultiplyAdd<
// CHECK610-NOT: MakeInterpretedVector

RWStructuredBuffer<int32_t> outputBuffer;
ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let packedVec = coopVecLoad<1, uint32_t>(input);
    let vec = coopVecLoad<4, int8_t>(input);

    let resultMatMul = coopVecMatMulPacked<int32_t, 4>(
        packedVec,
        CoopVecComponentType::SignedInt8Packed,
        4,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultMatMulAdd = coopVecMatMulAddPacked<int32_t, 4>(
        packedVec,
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
        4);

    let resultMatMulNonPacked = coopVecMatMul<int32_t, 4, 4>(
        vec,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultMatMulAddNonPacked = coopVecMatMulAdd<int32_t, 4, 4>(
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

    outputBuffer[0] = resultMatMul[0] + resultMatMulAdd[0] + resultMatMulNonPacked[0] +
        resultMatMulAddNonPacked[0];
}
