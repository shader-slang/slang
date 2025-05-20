//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=4
//-render-feature cooperative-matrix-per-element-operations

//CHECK:type: int32_t
//CHECK-NEXT:9
//CHECK-NEXT:12
//CHECK-NEXT:15
//CHECK-NEXT:14

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input1
StructuredBuffer<int> input1;

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4),name=input2
StructuredBuffer<int> input2;

//TEST_INPUT:ubuffer(data=[2 3 4 1], stride=4),name=input3
StructuredBuffer<int> input3;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int MapOp(uint32_t row, uint32_t col, int a, int b, int c)
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
    let f = ((uint32_t x, uint32_t y, int a, int b, int c) => a + b + c + 1 + 2 + 3);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 2
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, int a, int b, int c) => a + b + c + 1 + 2 + 3);

#elif TEST_MODE == 3
    let f = ((uint32_t x, uint32_t y, int a, int b, int c) => a + b + c + c0 + c1 + c2);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 4
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, int a, int b, int c) => a + b + c + c0 + c1 + c2);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
