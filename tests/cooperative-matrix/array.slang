//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation

// CHECK: type: float
// CHECK: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 8.000000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=256),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[5.0 6.0 7.0 8.0], stride=256),name=input1
ByteAddressBuffer input2;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    CoopMatType coopMatArray[2];
    coopMatArray[0] = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    coopMatArray[1] = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);

    coopMatArray[0].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
    coopMatArray[1].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 4, stride);
}
