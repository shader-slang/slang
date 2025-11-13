//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0 -render-feature cooperative-matrix
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1 -render-feature cooperative-matrix
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2 -render-feature cooperative-matrix
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3 -render-feature cooperative-matrix
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=4 -render-feature cooperative-matrix

//CHECK:type: float16_t
//CHECK-NEXT:9
//CHECK-NEXT:12
//CHECK-NEXT:15
//CHECK-NEXT:14

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=2),name=input1
StructuredBuffer<float16_t> input1;

//TEST_INPUT:ubuffer(data=[0.0 1.0 2.0 3.0], stride=2),name=input2
StructuredBuffer<float16_t> input2;

//TEST_INPUT:ubuffer(data=[2.0 3.0 4.0 1.0], stride=2),name=input3
StructuredBuffer<float16_t> input3;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<float16_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<float16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

float16_t MapOp(uint32_t row, uint32_t col, float16_t a, float16_t b, float16_t c)
{
    return a + b + c + 1.0 + 2.0 + 3.0;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);
    let mat2 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input2, 0, stride);
    let mat3 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input3, 0, stride);

    // Testing the capturing lambda
    float16_t c0 = 1.0;
    float16_t c1 = 2.0;
    float16_t c2 = 3.0;

    CoopMatType result;

#if TEST_MODE == 0
    result = makeTuple(mat1, mat2, mat3).MapElement(MapOp);

#elif TEST_MODE == 1
    let f = ((uint32_t x, uint32_t y, float16_t a, float16_t b, float16_t c) => a + b + c + 1.0 + 2.0 + 3.0);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 2
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, float16_t a, float16_t b, float16_t c) => a + b + c + 1.0 + 2.0 + 3.0);

#elif TEST_MODE == 3
    let f = ((uint32_t x, uint32_t y, float16_t a, float16_t b, float16_t c) => a + b + c + c0 + c1 + c2);
    result = makeTuple(mat1, mat2, mat3).MapElement(f);

#elif TEST_MODE == 4
    result = makeTuple(mat1, mat2, mat3).MapElement((uint32_t x, uint32_t y, float16_t a, float16_t b, float16_t c) => a + b + c + c0 + c1 + c2);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
