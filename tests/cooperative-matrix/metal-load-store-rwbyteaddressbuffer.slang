//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// CHECK: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, count=256):name=inputBuffer
ByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 8;
    let mat = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0u, 8u);
}
