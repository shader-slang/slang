//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// MMA m16n8k16 with global memory load/store
// A: 16x16 (half), A[row][col] = row (0..15)
// B: 16x8  (half), all 1.0
// C: 16x8  (float), all 0.0
// D[i][j] = sum(A[i][k] * B[k][j], k=0..15) + 0 = i * 16
//
// Row 0:  0.0, Row 1: 16.0, Row 2: 32.0, ... Row 15: 240.0

// CHECK: type: float
// CHECK-COUNT-8: 0.0
// CHECK-COUNT-8: 16.0
// CHECK-COUNT-8: 32.0
// CHECK-COUNT-8: 48.0
// CHECK-COUNT-8: 64.0
// CHECK-COUNT-8: 80.0
// CHECK-COUNT-8: 96.0
// CHECK-COUNT-8: 112.0
// CHECK-COUNT-8: 128.0
// CHECK-COUNT-8: 144.0
// CHECK-COUNT-8: 160.0
// CHECK-COUNT-8: 176.0
// CHECK-COUNT-8: 192.0
// CHECK-COUNT-8: 208.0
// CHECK-COUNT-8: 224.0
// CHECK-COUNT-8: 240.0

//TEST_INPUT:ubuffer(stride=2, count=256),name=inputA
RWStructuredBuffer<half> inputA;

//TEST_INPUT:ubuffer(stride=2, count=128),name=inputB
RWStructuredBuffer<half> inputB;

//TEST_INPUT:ubuffer(stride=4, count=128):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // Fill A[row][col] = half(row), B = all 1.0h
    for (uint i = tid; i < 256; i += 32)
        inputA[i] = half(float(i / 16));
    for (uint i = tid; i < 128; i += 32)
        inputB[i] = half(1.0);

    GroupMemoryBarrierWithGroupSync();

    // Load A and B from global memory
    let matA = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.RowMajor>(inputA, 0, 16);
    let matB = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.RowMajor>(inputB, 0, 8);

    // D = A * B + 0
    let result = coopMatMulAdd<float, false>(
        matA,
        matB,
        CoopMat<float, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixAccumulator>(0.0));

    // Store result to global memory
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, 8);
}
