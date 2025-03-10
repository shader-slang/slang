//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8]):name=input
RWByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0]):out,name=outputBuffer
RWStructuredBuffer<uint32_t> outputBuffer;

groupshared uint32_t[8] tempShared;

typealias CoopMatType = CoopMat<uint32_t, CoopMatScope::Subgroup, 2, 4, CoopMatMatrixUse::A>;

[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat = CoopMatType.load(input, 0, stride, matrixLayout);
    mat.store(tempShared, 0, stride, matrixLayout);

    let result = CoopMatType.load(tempShared, 0, stride, matrixLayout);
    result.store(outputBuffer, 0, stride, matrixLayout);
}
