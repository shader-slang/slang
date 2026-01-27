//TEST_DISABLED(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK-VK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK-CUDA):-cuda -output-using-type -capability cuda_sm_8_0

// CHECK-VK: type: half
// CHECK-VK-COUNT-256: 241.0

// CHECK-CUDA: type: half
// CHECK-CUDA-COUNT-256: 241.0

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer1
RWStructuredBuffer<half> outputBuffer1;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<half, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0U, 16U);
}
