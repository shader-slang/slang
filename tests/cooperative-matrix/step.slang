//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: float
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[2.0 3.0 4.0 5.0], stride=4),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[1.0 2.0 5.0 6.0], stride=4),name=input2
ByteAddressBuffer input2;

typealias CoopMatType = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>;

[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let edge = CoopMatType.load(input1, 0, stride, matrixLayout);
    let x = CoopMatType.load(input2, 0, stride, matrixLayout);

    CoopMatType result = step(edge, x);
    result.store(outputBuffer, 0, 4, matrixLayout);
}
