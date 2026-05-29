//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -capability optix_coopvec -entry computeMain -stage compute

using namespace linalg;

ByteAddressBuffer matrix;
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    let packedVec = coopVecLoad<1, uint32_t>(matrix);
    let result = coopVecMatMulPacked<int32_t, 4, 1>(
        packedVec,
        CoopVecComponentType::SignedInt8Packed,
        4,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    outputBuffer[0] = result[0];
}

//CHECK: unsupported intrinsic operation
//CHECK: intrinsic operation 'cooperative vector matrix multiply-add with packed input is not implemented yet' is not supported for the current target.
