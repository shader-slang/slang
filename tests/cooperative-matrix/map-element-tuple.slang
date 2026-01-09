//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3 -render-feature cooperative-matrix-per-element-operations
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=4 -render-feature cooperative-matrix-per-element-operations

//CHECK:type: float
//CHECK-NEXT:9
//CHECK-NEXT:12
//CHECK-NEXT:15
//CHECK-NEXT:14

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input1
StructuredBuffer<float> input1;

//TEST_INPUT:ubuffer(data=[0.0 1.0 2.0 3.0], stride=4),name=input2
StructuredBuffer<float> input2;

//TEST_INPUT:ubuffer(data=[2.0 3.0 4.0 1.0], stride=4),name=input3
StructuredBuffer<float> input3;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

float MapOp(uint32_t row, uint32_t col, float a, float b, float c)
{
    return a + b + c + 1 + 2 + 3;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);
    let mat2 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input2, 0, stride);
    let mat3 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input3, 0, stride);

    // Testing the capturing lambda
    int c0 = 1;
    int c1 = 2;
    int c2 = 3;

    CoopMatType result;

#if TEST_MODE == 0
    result = makeTuple(mat1, mat2, mat3).MapElement(MapOp);

#elif TEST_MODE == 1
    let f = ((uint32_t x, uint32_t y, float a, float b, float c) => a + b + c + 1 + 2 + 3);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 2
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, float a, float b, float c) => a + b + c + 1 + 2 + 3);
#elif TEST_MODE == 3
    let f = ((uint32_t x, uint32_t y, float a, float b, float c) => a + b + c + c0 + c1 + c2);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 4
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, float a, float b, float c) => a + b + c + c0 + c1 + c2);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
