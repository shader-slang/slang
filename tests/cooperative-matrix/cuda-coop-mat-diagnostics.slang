// Test that CUDA cooperative matrix emitter rejects unsupported element types,
// invalid shapes, and invalid (A, B, C, D) type combinations for `coopMatMulAdd`.

// Test 1: double element type is not supported on CUDA (only half / bfloat16 /
// int8 / uint8 / FloatE4M3 / FloatE5M2 are valid for MatrixA / MatrixB).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_TYPE, non-exhaustive): -target cuda -entry testUnsupportedType -stage compute

// Test 2: invalid shape (8x16 instead of 16x16) for MatrixA on CUDA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SHAPE, non-exhaustive): -target cuda -entry testInvalidShape -stage compute

// Test 3: invalid (A, B, C, D) type combination — A=int8 but B=half is not a
// valid pairing for CUDA mma.  Without this check, the user would only see a
// confusing C++ template error from NVRTC at PTX-compile time.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COMBO, non-exhaustive): -target cuda -capability cuda_sm_8_0 -entry testInvalidCombination -stage compute

// CHECK_TYPE: error 50000: unsupported element type for cooperative matrix

// CHECK_SHAPE: error 50000: invalid shape for cooperative matrix

// CHECK_COMBO: error 50000: invalid type combination for cooperative matrix multiply-add

using namespace linalg;

RWStructuredBuffer<int32_t> outputBuffer;

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
void testInvalidCombination()
{
    coopMatMulAdd<int32_t, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(int8_t(3)),
        CoopMat<half,   MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(half(5.0)),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
