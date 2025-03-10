//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 2.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[5.0 4.0 3.0 2.0], stride=4),name=input2
ByteAddressBuffer input2;

typealias CoopMatType = CoopMat<int32_t, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>;

[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat1 = CoopMatType.load(input1, 0, stride, matrixLayout);
    let mat2 = CoopMatType.load(input2, 0, stride, matrixLayout);

    let result = max(mat1, mat2);
    result.store(outputBuffer, 0, 4, matrixLayout);
}
