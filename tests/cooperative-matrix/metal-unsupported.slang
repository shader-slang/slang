//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COUNT,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_GETCOUNT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUBSCRIPT,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_SUBSCRIPT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_LENGTH,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_GETLENGTH
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SCALAR_MUL,non-exhaustive): -target metal -stage compute -entry computeMain -DTEST_SCALAR_MUL

// Verify the compiler rejects unsupported CoopMat operations on Metal.
// Metal's simdgroup_matrix only supports fill, Load, Store, and coopMatMulAdd.

using namespace linalg;

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
}
