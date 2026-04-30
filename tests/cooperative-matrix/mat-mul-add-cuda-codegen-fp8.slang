//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -capability cuda_sm_8_9
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -capability cuda_sm_8_9 -entry computeMain -stage compute

// Smoke test: e4m3 x e4m3 -> float and e5m2 x e5m2 -> half cooperative matrix
// multiply-add on CUDA emit Slang_CUDA_WMMA::coopMatMulAdd specialized on the
// __nv_fp8_e4m3 / __nv_fp8_e5m2 input types and lower to
// mma.sync.aligned.m16n8k16.row.col.{f32|f16}.{e4m3|e5m2}.{e4m3|e5m2}.{f32|f16}.

// CHECK-DAG: Slang_CUDA_WMMA::WmmaFragment<__nv_fp8_e4m3
// CHECK-DAG: Slang_CUDA_WMMA::WmmaFragment<__nv_fp8_e5m2

// CHECK-PTX-DAG: mma.sync.aligned.m16n8k16.row.col.f32.e4m3.e4m3.f32
// CHECK-PTX-DAG: mma.sync.aligned.m16n8k16.row.col.f16.e5m2.e5m2.f16

RWStructuredBuffer<float> outputBuffer1;
RWStructuredBuffer<half>  outputBuffer2;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    // e4m3 inputs, f32 accumulator
    coopMatMulAdd<float, false>(
        CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(FloatE4M3(1.0)),
        CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(FloatE4M3(2.0)),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0, 16);

    // e5m2 inputs, f16 accumulator
    coopMatMulAdd<half, false>(
        CoopMat<FloatE5M2, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(FloatE5M2(1.0)),
        CoopMat<FloatE5M2, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(FloatE5M2(2.0)),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(half(3.0))
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer2, 0, 16);
}
