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

// m16n16k16 only (mma.sync.m16n8k16 register layout): 4 regs for half A/B, 8 regs for f32 accum
//CHK_CUDA:8
//CHK_CUDA:8
//CHK_CUDA:8
//CHK_CUDA:8

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
        outputBuffer[3] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>.GetLength();

    case spirv:
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse.MatrixA>.GetLength();
        outputBuffer[3] = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>.GetLength();

    case cuda:
        // m16n16k16: 4 regs for half A (mma.sync.m16n8k16 layout), 2 elements per reg = 8
        outputBuffer[1] = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>.GetLength();

        // m16n16k16: 2 regs for int8 A, 4 elements per reg = 8
        outputBuffer[2] = CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>.GetLength();

        // m16n16k16: 4 regs for half B, 2 elements per reg = 8
        outputBuffer[3] = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>.GetLength();
    }
}

