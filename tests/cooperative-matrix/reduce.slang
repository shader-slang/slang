//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_ROW):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=0 -render-feature cooperative-matrix-reductions
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_COLUMN):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=1 -render-feature cooperative-matrix-reductions
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_ROW_AND_COLUMN):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=2 -render-feature cooperative-matrix-reductions
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_2X2):-vk -output-using-type -emit-spirv-directly -Xslang -DTEST_MODE=3 -render-feature cooperative-matrix-reductions

//CHK_ROW-COUNT-8: 36

//CHK_COLUMN:1
//CHK_COLUMN-NEXT:2
//CHK_COLUMN-NEXT:3
//CHK_COLUMN-NEXT:4
//CHK_COLUMN-NEXT:5
//CHK_COLUMN-NEXT:6
//CHK_COLUMN-NEXT:7
//CHK_COLUMN-NEXT:8

//CHK_ROW_AND_COLUMN-COUNT-256: 36

//CHK_2X2:3
//CHK_2X2:7
//CHK_2X2:11
//CHK_2X2:15

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h], stride=2, count=256),name=inputBuffer
//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer

#if defined(BAB)
ByteAddressBuffer inputBuffer;
RWByteAddressBuffer outputBuffer;
#else
StructuredBuffer<half> inputBuffer;
RWStructuredBuffer<half> outputBuffer;
#endif

using namespace linalg;

half CombineOp(half lhs, half rhs)
{
    return lhs + rhs;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
#if TEST_MODE == 0
    let result = mat.ReduceRow<8>(CombineOp);
#elif TEST_MODE == 1
    let result = mat.ReduceColumn<16>(CombineOp);
#elif TEST_MODE == 2
    let result = mat.ReduceRowAndColumn<16, 16>(CombineOp);
#elif TEST_MODE == 3
    let result = mat.Reduce2x2(CombineOp);
#endif
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
