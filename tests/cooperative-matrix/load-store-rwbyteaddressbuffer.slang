//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

typealias CoopMatType = CoopMat<int32_t, CoopMatScope::Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat = CoopMatType.load(inputBuffer, 0, stride, matrixLayout);
    mat.store(outputBuffer, 0, 16, matrixLayout);
}
