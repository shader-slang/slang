// Test that CUDA cooperative matrix emitter rejects unsupported element types,
// invalid shapes, and invalid (A, B, C, D) type combinations for `coopMatMulAdd`.

// Test 1: double element type is not supported on CUDA (only half / bfloat16 /
// int8 / uint8 / FloatE4M3 / FloatE5M2 are valid for MatrixA / MatrixB).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_TYPE, non-exhaustive): -target cuda -entry testUnsupportedType -stage compute

// Test 2: invalid shape (8x16 instead of 16x16) for MatrixA on CUDA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SHAPE, non-exhaustive): -target cuda -entry testInvalidShape -stage compute

// Test 3a: invalid (A, B, C, D) type combination — AType != BType
// (A=int8 but B=half is not a valid pairing for CUDA mma).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COMBO, non-exhaustive): -target cuda -capability cuda_sm_8_0 -entry testInvalidCombinationMismatchedAB -stage compute

// Test 3b: invalid accumulator family — int8/int8 inputs with a half accumulator
// (integer mma only supports an int32 accumulator).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COMBO, non-exhaustive): -target cuda -capability cuda_sm_8_0 -entry testInvalidCombinationInt8HalfAcc -stage compute

// Test 3c: invalid accumulator family — bfloat16/bfloat16 inputs with an int
// accumulator (bf16 mma only supports a float accumulator).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COMBO, non-exhaustive): -target cuda -capability cuda_sm_8_0 -entry testInvalidCombinationBf16IntAcc -stage compute

// Test 3d: invalid accumulator family — fp8/fp8 inputs with mixed half C / float
// D (the prelude's fp8 helpers only specialize CType == DType).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COMBO, non-exhaustive): -target cuda -capability cuda_sm_8_9 -entry testInvalidCombinationFp8MixedAccOutput -stage compute

// CHECK_TYPE: error 50000: unsupported element type for cooperative matrix

// CHECK_SHAPE: error 50000: invalid shape for cooperative matrix

// CHECK_COMBO: error 50000: invalid type combination for cooperative matrix multiply-add

using namespace linalg;

RWStructuredBuffer<int32_t> outputBuffer;
RWStructuredBuffer<float>   outputBufferF;
RWStructuredBuffer<half>    outputBufferH;

[shader("compute")]
[numthreads(32, 1, 1)]
void testUnsupportedType()
{
    var mat = CoopMat<double, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(double(1.0));
    outputBuffer[0] = int32_t(mat.GetLength());
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidShape()
{
    var mat = CoopMat<half, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse::MatrixA>(half(1.0));
    outputBuffer[0] = int32_t(mat.GetLength());
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidCombinationMismatchedAB()
{
    // AType (int8) != BType (half) — both must share the same element type.
    coopMatMulAdd<int32_t, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(int8_t(3)),
        CoopMat<half,   MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(half(5.0)),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidCombinationInt8HalfAcc()
{
    // int8 inputs require an int32 accumulator; half is not allowed.
    coopMatMulAdd<half, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(int8_t(3)),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(int8_t(5)),
        CoopMat<half,   MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(half(1.0))
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBufferH, 0, 16);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidCombinationBf16IntAcc()
{
    // bf16 inputs require a float accumulator; int is not allowed.
    coopMatMulAdd<int32_t, false>(
        CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(BFloat16(3.0)),
        CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(BFloat16(5.0)),
        CoopMat<int32_t,  MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidCombinationFp8MixedAccOutput()
{
    // The prelude's fp8 helpers only specialize CType == DType.  Mixing them
    // (half C, float D) hits the validator's "fp8 needs CType == DType" rule.
    coopMatMulAdd<float, false>(
        CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(FloatE4M3(1.0)),
        CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(FloatE4M3(2.0)),
        CoopMat<half,      MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(half(3.0))
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBufferF, 0, 16);
}
