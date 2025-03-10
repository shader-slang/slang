//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// Hilariously low precision because the different APIs don't agree (they're
// not specced to of course, so it's ok)
// CHECK: type: float
// CHECK-NEXT: 0.761
// CHECK-NEXT: 0.964
// CHECK-NEXT: 0.995
// CHECK-NEXT: 0.999

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input
ByteAddressBuffer input;

typealias CoopMatType = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>;

[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat = CoopMatType.load(input, 0, stride, matrixLayout);

    let result = tanh(mat);
    result.store(outputBuffer, 0, 4, matrixLayout);
}
