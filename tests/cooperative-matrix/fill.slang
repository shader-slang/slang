//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: int32_t
// CHECK-NEXT: 10
// CHECK-NEXT: 10
// CHECK-NEXT: 10
// CHECK-NEXT: 10

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var result : CoopMat<int32_t, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>;
    result.fill(10);
    result.store(outputBuffer, 0, 4, CoopMatMatrixLayout::RowMajor);
}

