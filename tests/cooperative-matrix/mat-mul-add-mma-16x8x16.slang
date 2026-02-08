//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type  -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

// CHECK: type: half
// CHECK-NEXT: 1498.000000
// CHECK-NEXT: 3162.000000

//TEST_INPUT:ubuffer(stride=2, count=256):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

//TEST_INPUT:ubuffer(stride=2, count=128):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    // Initialize the input buffer with 256 elements
    for (int i = threadID.x; i < 256; i += 32)
    {
        inputBuffer[i] = (half)((i+1) % 32);
    }

    // Matrix A: 16x16 in row major
    let matA = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, 16);
    // Matrix B: 16x8 in column major
    let matB = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.ColumnMajor>(inputBuffer, 0, 16);
    // Matrix C: 16x8 in row major
    let matC = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixAccumulator>(2.0);
    let matD = coopMatMulAdd<half, false>(matA, matB, matC);

    matD.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0U, 8U);
}