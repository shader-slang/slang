//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-conversions

//CHECK: type: int32_t
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

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256),name=buf
StructuredBuffer<int32_t> inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    let result = mat.Transpose();
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
