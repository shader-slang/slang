// Requires VK_NV_cooperative_matrix2 extension (Transpose operation)
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix

//CHECK: type: float16_t
//CHECK: 1
//CHECK-COUNT-15: 0
//CHECK: 2
//CHECK-COUNT-15: 0
//CHECK: 3
//CHECK-COUNT-15: 0
//CHECK: 4
//CHECK-COUNT-15: 0
//CHECK: 5
//CHECK-COUNT-15: 0
//CHECK: 6
//CHECK-COUNT-15: 0
//CHECK: 7
//CHECK-COUNT-15: 0
//CHECK: 8
//CHECK-COUNT-15: 0

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=2, count=256),name=buf
StructuredBuffer<float16_t> inputBuffer;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<float16_t> outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<float16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    let result = mat.Transpose();
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
