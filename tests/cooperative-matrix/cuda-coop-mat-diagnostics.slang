// Test that CUDA cooperative matrix emitter rejects unsupported element types and invalid shapes.

// Test 1: int8 element type is not supported for MatrixA on CUDA (only half/bfloat16 allowed)
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_TYPE, non-exhaustive): -target cuda -entry testUnsupportedType -stage compute

// Test 2: invalid shape (8x16 instead of 16x16) for MatrixA on CUDA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SHAPE, non-exhaustive): -target cuda -entry testInvalidShape -stage compute

// CHECK_TYPE: error 50000: unsupported element type for cooperative matrix

// CHECK_SHAPE: error 50000: invalid shape for cooperative matrix

using namespace linalg;

RWStructuredBuffer<int32_t> outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void testUnsupportedType()
{
    coopMatMulAdd<int32_t, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void testInvalidShape()
{
    var mat = CoopMat<half, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse::MatrixA>(half(1.0));
    outputBuffer[0] = int32_t(mat.GetLength());
}
