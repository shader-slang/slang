//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COUNT,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_GETCOUNT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUBSCRIPT,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_SUBSCRIPT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_LENGTH,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_GETLENGTH
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SCALAR_MUL,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_SCALAR_MUL
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BAD_DIM,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_BAD_DIM
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BAD_TYPE,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_BAD_TYPE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SAT_ACCUM,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_SAT_ACCUM

// Verify the compiler rejects unsupported CoopMat operations on Metal.
// Metal's simdgroup_matrix only supports fill, Load, Store, and coopMatMulAdd.

using namespace linalg;

RWStructuredBuffer<float> outputBuffer;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
    var mat : CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;
    mat.fill(1.0);

#ifdef TEST_GETCOUNT
    // CHECK_COUNT: error 36107
    // CHECK_COUNT: see using of 'getCount'
    let count = mat.getCount();
#endif

#ifdef TEST_SUBSCRIPT
    // CHECK_SUBSCRIPT: error 36107
    // CHECK_SUBSCRIPT: see using of 'operator[]'
    let val = mat[0];
#endif

#ifdef TEST_GETLENGTH
    // CHECK_LENGTH: error 36107
    // CHECK_LENGTH: see using of 'GetLength'
    let len = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>.GetLength();
#endif

#ifdef TEST_SCALAR_MUL
    // CHECK_SCALAR_MUL: error 36107
    // CHECK_SCALAR_MUL: see using of '*'
    let scaled = mat * 2.0;
#endif

#ifdef TEST_BAD_DIM
    // CHECK_BAD_DIM: error 99999
    // CHECK_BAD_DIM: only support 8x8 dimensions
    var mat16 : CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;
    mat16.fill(1.0);
    mat16.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0u, 16u);
#endif

#ifdef TEST_BAD_TYPE
    // CHECK_BAD_TYPE: error 99999
    // CHECK_BAD_TYPE: only support half and float
    var matInt : CoopMat<int32_t, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;
    matInt.fill(1);
    matInt.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0u, 8u);
#endif

#ifdef TEST_SAT_ACCUM
    // CHECK_SAT_ACCUM: error 99999
    // CHECK_SAT_ACCUM: saturating accumulation
    coopMatMulAdd<float, true>(
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixA>(2.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixB>(3.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0u, 8u);
#endif
}
