//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type  -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

// CHECK: type: half
// CHECK-COUNT-128: 241.0

// TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<half, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixB>(5.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ) .Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0U, 8U);


}