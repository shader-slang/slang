//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_9

// CHECK: ByteAddressBuffer matrix_0 : register
// CHECK: ByteAddressBuffer bias_0 : register
// CHECK: __builtin_MatVecMul(_S{{[0-9]+}}, false, _S{{[0-9]+}}, true, 17, matrix_0, int(0), 20, int(4), int(4), int(0), false, 4U);
// CHECK: __builtin_MatVecMulAdd(_S{{[0-9]+}}, false, _S{{[0-9]+}}, true, 17, matrix_0, int(0), 20, int(4), int(4), int(0), false, 4U, bias_0, int(0), 4);

RWStructuredBuffer<int32_t> outputBuffer;
ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let vec = coopVecLoad<1, uint32_t>(input);

    let resultMatMul = coopVecMatMulPacked<int32_t, 4>(
        vec,
        CoopVecComponentType::SignedInt8Packed,
        4,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultMatMulAdd = coopVecMatMulAddPacked<int32_t, 4>(
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
        4);

    outputBuffer[0] = resultMatMul[0] + resultMatMulAdd[0];
}
