//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// (2.0 * 3.0) * 8 + 1.0 = 49.0
// CHECK: type: float
// CHECK-COUNT-64: 49.0

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<float, false>(
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixA>(2.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixB>(3.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0u, 8u);
}
