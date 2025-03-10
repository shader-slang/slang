//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: float
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input
ByteAddressBuffer input;


[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let intMat = CoopMat<int, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>.load(input, 0, stride, matrixLayout);
    let floatMat = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>(intMat);
    let uintMat = CoopMat<uint, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>(intMat);
    let floatMat2 = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>(uintMat);

    let result = floatMat + floatMat2;
    result.store(outputBuffer, 0, stride, matrixLayout);
}
