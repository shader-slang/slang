//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_9 -entry computeMain

// Test that CoopVecMatrixLayout enum values are emitted as named dx::linalg
// constants in HLSL output rather than integer casts.
//
// The Slang enum name -> dx::linalg constant mapping is:
//   RowMajor            -> MATRIX_LAYOUT_ROW_MAJOR
//   ColumnMajor         -> MATRIX_LAYOUT_COLUMN_MAJOR
//   InferencingOptimal  -> MATRIX_LAYOUT_MUL_OPTIMAL
//   TrainingOptimal     -> MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL

//HLSL: MATRIX_LAYOUT_ROW_MAJOR
//HLSL: MATRIX_LAYOUT_COLUMN_MAJOR
//HLSL: MATRIX_LAYOUT_MUL_OPTIMAL
//HLSL: MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL

ByteAddressBuffer inputBuf;
ByteAddressBuffer matBuf;
RWByteAddressBuffer rwMatBuf;
RWStructuredBuffer<float16_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let vec = coopVecLoad<4, float16_t>(inputBuf);

    let r0 = coopVecMatMul<float16_t, 4, 4>(
        vec, CoopVecComponentType::Float16,
        matBuf, 0, CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor, false, 8);

    let r1 = coopVecMatMul<float16_t, 4, 4>(
        vec, CoopVecComponentType::Float16,
        matBuf, 0, CoopVecComponentType::Float16,
        CoopVecMatrixLayout::ColumnMajor, false, 8);

    let r2 = coopVecMatMul<float16_t, 4, 4>(
        vec, CoopVecComponentType::Float16,
        matBuf, 0, CoopVecComponentType::Float16,
        CoopVecMatrixLayout::InferencingOptimal, false, 8);

    coopVecOuterProductAccumulate<float16_t, 4, 4>(
        vec, vec,
        rwMatBuf, 0, 8,
        CoopVecMatrixLayout::TrainingOptimal,
        CoopVecComponentType::Float16);

    for (int i = 0; i < 4; ++i)
        outputBuffer[i] = r0[i] + r1[i] + r2[i];
}
