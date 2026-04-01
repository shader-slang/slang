//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_VK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK_CUDA):-cuda

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include
//DXIL: result code = 0

// Note the length is NOT row * column.
// When the memory scope is set to subgroup, each thread gets 16 * 16 / 32 = 8 where 32 is the value used in `numthreads`.

//CHK_VK:8
//CHK_VK:4
//CHK_VK:10
//CHK_VK:4

// https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-fragment
//CHK_CUDA:8
//CHK_CUDA:10
//CHK_CUDA:10
//CHK_CUDA:10

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
    case hlsl:
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[2] = 0;// int8 it not supported by HLSL

    case spirv:
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse.MatrixA>.GetLength();

    case cuda:
        // for f16 type, with any shapes, each fragment of A and B is a vector of 8 f16x2 registers, each registers contains 2 elements.
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse.MatrixA>.GetLength();

        // for int8 type, m32n8k16 shape, each fragment of A is a vector of 4 b32 registers, each registers contains 4 elements.
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse.MatrixA>.GetLength();
    }

    outputBuffer[3] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>.GetLength();
}

