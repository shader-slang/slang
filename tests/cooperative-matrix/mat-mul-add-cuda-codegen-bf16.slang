//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

// Smoke test: bf16 x bf16 -> float cooperative matrix multiply-add on CUDA
// emits a Slang_CUDA_WMMA::coopMatMulAdd specialized on __nv_bfloat16 inputs.

// CHECK: Slang_CUDA_WMMA::coopMatMulAdd<
// CHECK-SAME: __nv_bfloat16
// CHECK-SAME: __nv_bfloat16

RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<float, false>(
        CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(BFloat16(3.0)),
        CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(BFloat16(5.0)),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
