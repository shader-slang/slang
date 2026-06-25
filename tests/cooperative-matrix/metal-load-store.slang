//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// CHECK: 1.00
// CHECK-NEXT: 2.00
// CHECK-NEXT: 3.00
// CHECK-NEXT: 4.00
// CHECK-NEXT: 5.00
// CHECK-NEXT: 6.00
// CHECK-NEXT: 7.00
// CHECK-NEXT: 8.00
// CHECK-NEXT: 9.00
// CHECK-NEXT: 10.00
// CHECK-NEXT: 11.00
// CHECK-NEXT: 12.00
// CHECK-NEXT: 13.00
// CHECK-NEXT: 14.00
// CHECK-NEXT: 15.00
// CHECK-NEXT: 16.00

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h 9.0h 10.0h 11.0h 12.0h 13.0h 14.0h 15.0h 16.0h], stride=2, count=256):name=input
RWByteAddressBuffer input;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 8;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(input, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
