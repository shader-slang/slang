//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type -render-feature cooperative-matrix-conversions

//CHECK: type: half
//CHECK: 1.00
//CHECK-COUNT-15: 0.00
//CHECK: 2.00
//CHECK-COUNT-15: 0.00
//CHECK: 3.00
//CHECK-COUNT-15: 0.00
//CHECK: 4.00
//CHECK-COUNT-15: 0.00
//CHECK: 5.00
//CHECK-COUNT-15: 0.00
//CHECK: 6.00
//CHECK-COUNT-15: 0.00
//CHECK: 7.00
//CHECK-COUNT-15: 0.00
//CHECK: 8.00
//CHECK-COUNT-15: 0.00

//TEST_INPUT:ubuffer(data=[1.h 2.h 3.h 4.h 5.h 6.h 7.h 8.h], stride=2, count=256),name=buf
StructuredBuffer<half> inputBuffer;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    let result = mat.Transpose();
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
