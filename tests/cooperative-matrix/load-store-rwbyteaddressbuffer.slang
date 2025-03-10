//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8]):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

typealias CoopMatType = CoopMat<int32_t, CoopMatScope::Subgroup, 2, 4, CoopMatMatrixUse::A>;

[numthreads(1, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat = CoopMatType.load(inputBuffer, 0, stride, matrixLayout);
    mat.store(outputBuffer, 0, 4, matrixLayout);
}
