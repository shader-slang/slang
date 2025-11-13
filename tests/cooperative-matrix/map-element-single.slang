// Requires VK_NV_cooperative_matrix2 extension (cooperativeMatrixPerElementOperations)
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0 -render-feature cooperative-matrix
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1 -render-feature cooperative-matrix
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2 -render-feature cooperative-matrix
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3 -render-feature cooperative-matrix
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=4 -render-feature cooperative-matrix

//CHECK: type: float16_t
//CHECK-NEXT: 8
//CHECK-NEXT: 10
//CHECK-NEXT: 12
//CHECK-NEXT: 14

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=2),name=input1
StructuredBuffer<float16_t> input1;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<float16_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<float16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

float16_t MapOp(uint32_t row, uint32_t col, float16_t value)
{
    return value * 2.0 + 1.0 + 2.0 + 3.0;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    CoopMatType mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);

    // Testing the capturing lambda
    float16_t c0 = 1.0;
    float16_t c1 = 2.0;
    float16_t c2 = 3.0;

    CoopMatType result;

#if TEST_MODE == 0
    result = mat1.MapElement(MapOp);

#elif TEST_MODE == 1
    // Lambda via a temp variable (no capture)
    let func = ((uint32_t row, uint32_t column, float16_t value) => value * 2.0 + 1.0 + 2.0 + 3.0);
    result = mat1.MapElement(func);

#elif TEST_MODE == 2
    // Directly use lambda (no capture)
    result = mat1.MapElement((uint32_t row, uint32_t column, float16_t value) => value * 2.0 + 1.0 + 2.0 + 3.0);

#elif TEST_MODE == 3
    // Lambda via a temp variable (capture)
    let func = ((uint32_t row, uint32_t column, float16_t value) => value * 2.0 + c0 + c1 + c2);
    result = mat1.MapElement(func);

#elif TEST_MODE == 4
    // Directly use lambda (capture)
    result = mat1.MapElement((uint32_t row, uint32_t column, float16_t value) => value * 2.0 + c0 + c1 + c2);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
