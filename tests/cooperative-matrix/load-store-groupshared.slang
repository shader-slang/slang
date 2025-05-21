//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=input
ByteAddressBuffer input;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<uint32_t> outputBuffer;

using namespace linalg;

groupshared uint32_t[256] tempShared;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let mat = coopMatLoad<uint32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(input, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(tempShared, 0, stride);

    let result = coopMatLoad<uint32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(tempShared, 0, stride);
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
