//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: int32_t
// CHECK-NEXT: 4
// CHECK-NEXT: 4
// CHECK-NEXT: 4
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let mat = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>(4.0);
    var result = CoopMat<int32_t, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>(0);
    result.copyFrom(mat);
    result.store(outputBuffer, 0, 4, CoopMatMatrixLayout::RowMajor);
}

