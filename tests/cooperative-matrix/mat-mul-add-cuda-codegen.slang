//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

// CHECK: Slang_CUDA_WMMA::coopMatMulAdd<

RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5.0),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
