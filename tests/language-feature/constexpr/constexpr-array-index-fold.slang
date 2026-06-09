// Test that array indexing into a `constexpr` array is constant-folded,
// so array elements can be used as generic arguments and array sizes.
// Tests the tryFoldIndexExpr path for ConstExprModifier.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

constexpr uint sizes[3] = { 2, 3, 4 };

[numthreads(1, 1, 1)]
void computeMain()
{
    // Use an array element as a compile-time generic argument.
    vector<uint, sizes[1]> v = { 10, 20, 30 };
    outputBuffer[0] = v[2];
}

// CHECK: 30
