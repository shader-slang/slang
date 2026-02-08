//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Test ColMajor layout for load and store operations
// Matrix data is stored column-by-column

// Expected output: First column of the 16x8 matrix
// CHECK: type: half
// CHECK: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 8.000000

//TEST_INPUT:ubuffer(stride=2, count=128):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=2, count=128):out,name=resultBuffer
RWStructuredBuffer<half> resultBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    // Initialize the input buffer with 128 elements
    // fill data with 1 to 32 repeating
    for (int i = threadID.x; i < 128; i += 32)
    {
        inputBuffer[i] = (half)((i+1) % 32);
    }

    // Load 16x8 matrix in column-major layout
    // stride = 16 (number of rows, since data is column-major)
    // Need to use CoopMatMatrixUse.MatrixB for column-major layout
    let stride = 16;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.ColumnMajor>(inputBuffer, 0, stride);

    // Store back in column-major layout
    mat.Store<CoopMatMatrixLayout.ColumnMajor>(resultBuffer, 0, stride);
}
