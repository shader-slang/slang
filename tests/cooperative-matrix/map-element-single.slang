//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=4 -render-feature cooperative-matrix-per-element-operations

//CHECK: type: int32_t
//CHECK-NEXT: 8
//CHECK-NEXT: 10
//CHECK-NEXT: 12
//CHECK-NEXT: 14

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input1
StructuredBuffer<int> input1;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int MapOp(uint32_t row, uint32_t col, int value)
{
    return value * 2 + 1 + 2 + 3;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    CoopMatType mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);

    // Testing the capturing lambda
    int c0 = 1;
    int c1 = 2;
    int c2 = 3;

    CoopMatType result;

#if TEST_MODE == 0
    result = mat1.MapElement(MapOp);

#elif TEST_MODE == 1
    // Lambda via a temp variable (no capture)
    let func = ((uint32_t row, uint32_t column, int value) => value * 2 + 1 + 2 + 3);
    result = mat1.MapElement(func);

#elif TEST_MODE == 2
    // Directly use lambda (no capture)
    result = mat1.MapElement((uint32_t row, uint32_t column, int value) => value * 2 + 1 + 2 + 3);

#elif TEST_MODE == 3
    // Lambda via a temp variable (capture)
    let func = ((uint32_t row, uint32_t column, int value) => value * 2 + c0 + c1 + c2);
    result = mat1.MapElement(func);

#elif TEST_MODE == 4
    // Directly use lambda (capture)
    result = mat1.MapElement((uint32_t row, uint32_t column, int value) => value * 2 + c0 + c1 + c2);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
