//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-per-element-operations -Xslang -DTEST_MODE=0
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-per-element-operations -Xslang -DTEST_MODE=1
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-per-element-operations -Xslang -DTEST_MODE=2

//CHECK: type: int32_t
//CHECK-NEXT: 2
//CHECK-NEXT: 4
//CHECK-NEXT: 6
//CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input1
StructuredBuffer<int> input1;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int MapOp(uint32_t row, uint32_t col, int value)
{
    return value * 2;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    CoopMatType mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);

    CoopMatType result;

#if TEST_MODE == 0
    result = mat1.MapElement(MapOp);

#elif TEST_MODE == 1
    // Lambda through IFunc.
    // TODO: Not working due to issue #7024
    IFunc<int, uint32_t, uint32_t, int> func = ((uint32_t row, uint32_t column, int value) => value * 2);
    result = mat1.MapElement(func);

#elif TEST_MODE == 2
    // Directly use lambda.
    // TODO: Not working due to issue #7024
    result = mat1.MapElement((uint32_t row, uint32_t column, int value) => (int)(value));
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
