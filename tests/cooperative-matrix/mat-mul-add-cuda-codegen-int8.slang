//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -capability cuda_sm_8_0 -entry computeMain -stage compute

// Smoke test: int8 x int8 -> int32 cooperative matrix multiply-add on CUDA
// emits a Slang_CUDA_WMMA::coopMatMulAdd specialized on `char` inputs and
// lowers to mma.sync.aligned.m16n8k16.row.col.s32.s8.s8.s32 in PTX.

// CHECK: Slang_CUDA_WMMA::coopMatMulAdd<
// CHECK-SAME: char
// CHECK-SAME: char

// CHECK-PTX: mma.sync.aligned.m16n8k16.row.col.s32.s8.s8.s32

RWStructuredBuffer<int> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<int, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(int8_t(3)),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(int8_t(5)),
        CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
