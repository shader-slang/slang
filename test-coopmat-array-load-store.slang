// Test: CoopMat Load/Store with groupshared U[V] array where U != T
//
// Compile with:
//   slangc test-coopmat-array-load-store.slang -target spirv-asm -o test.spv -stage compute -entry computeMain
//   slangc test-coopmat-array-load-store.slang -target cuda -o test.cu -stage compute -entry computeMain

using namespace linalg;

static const int SHMEM_SIZE = 128;

groupshared uint4 g_sharedMem[SHMEM_SIZE];

RWStructuredBuffer<float> outputBuffer;

[require(cooperative_matrix, subgroup_basic)]
[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    typealias MatA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
    typealias MatC = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

    // --- Test Load from groupshared uint4[] into CoopMat<half> (U=uint4, T=half) ---
    // Fill shared memory with some data first
    if (tid < SHMEM_SIZE)
        g_sharedMem[tid] = uint4(tid, tid + 1, tid + 2, tid + 3);
    GroupMemoryBarrierWithGroupSync();

    // Load CoopMat<half> from uint4[] array at element offset 0, stride = 2 (in uint4 units)
    MatA matA = MatA.Load<CoopMatMatrixLayout.RowMajor, uint4>(g_sharedMem, 16, 2);

    // --- Test Store CoopMat<float> into groupshared uint4[] (U=uint4, T=float) ---
    MatC matC;
    matC.fill(float(1.0));

    // Store at element offset 64 (second half of shared memory), stride = 2 (in uint4 units)
    matC.Store<CoopMatMatrixLayout.RowMajor, uint4>(g_sharedMem, 64, 2);

    GroupMemoryBarrierWithGroupSync();

    // Read back from shared memory to verify
    if (tid == 0)
    {
        outputBuffer[0] = float(g_sharedMem[64][0]);
    }
}
