//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -Xslang -DTEST_MODE=0
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -Xslang -DTEST_MODE=1

//CHECK:type: int32_t
//CHECK-NEXT:3
//CHECK-NEXT:6
//CHECK-NEXT:9
//CHECK-NEXT:8

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=input1
StructuredBuffer<int> input1;

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4),name=input2
StructuredBuffer<int> input2;

//TEST_INPUT:ubuffer(data=[2 3 4 1], stride=4),name=input3
StructuredBuffer<int> input3;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int MapOp(uint32_t row, uint32_t col, int a, int b, int c)
{
    return a + b + c;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);
    let mat2 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input2, 0, stride);
    let mat3 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input3, 0, stride);

    CoopMatType result;

#if TEST_MODE == 0
    result = makeTuple(mat1, mat2, mat3).MapElement(MapOp);
#else
    // TODO: Not working due to issue #7024
    result = makeTuple(mat1, mat2).MapElement((uint32_t x, uint32_t y, int a, int b, int c) => a + b + c);
#endif

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
