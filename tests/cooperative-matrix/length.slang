//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-cuda -output-using-type

// Note the length is NOT row * column.
// When the memory scope is set to subgroup, each thread gets 16 * 16 / 32 = 8 where 32 is the value used in `numthreads`.

//CHK:8
//CHK:4
//CHK:16
//CHK:4

//TEST_INPUT:ubuffer(stride=4, count=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    outputBuffer[0] = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>.GetLength();

    __target_switch
    {
    case spirv:
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse.MatrixA>.GetLength();

    case cuda:
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse.MatrixA>.GetLength();
    }

    outputBuffer[3] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>.GetLength();
}

