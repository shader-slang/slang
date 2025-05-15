//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_ROW):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-reductions -skip-spirv-validation -Xslang -DTEST_MODE=0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_COLUMN):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-reductions -skip-spirv-validation -Xslang -DTEST_MODE=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_ROW_AND_COLUMN):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-reductions -skip-spirv-validation -Xslang -DTEST_MODE=2
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_2X2):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix-reductions -skip-spirv-validation -Xslang -DTEST_MODE=3

//CHK_ROW-COUNT-8: 36

//CHK_COLUMN:1
//CHK_COLUMN-NEXT:2
//CHK_COLUMN-NEXT:3
//CHK_COLUMN-NEXT:4
//CHK_COLUMN-NEXT:5
//CHK_COLUMN-NEXT:6
//CHK_COLUMN-NEXT:7
//CHK_COLUMN-NEXT:8

//CHK_ROW_AND_COLUMN: 36

//CHK_2X2:3
//CHK_2X2:7
//CHK_2X2:11
//CHK_2X2:15

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256),name=buf
StructuredBuffer<int32_t> inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

int32_t CombineOp(int32_t lhs, int32_t rhs)
{
    return lhs + rhs;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
#if TEST_MODE == 0
    let result = mat.ReduceRow<8>(CombineOp);
#elif TEST_MODE == 1
    let result = mat.ReduceColumn<8>(CombineOp);
#elif TEST_MODE == 2
    let result = mat.ReduceRowAndColumn<8, 8>(CombineOp);
#elif TEST_MODE == 3
    let result = mat.Reduce2x2(CombineOp);
#endif
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
