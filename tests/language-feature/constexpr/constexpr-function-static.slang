// Test that function-local `static constexpr` variables are recognized as
// compile-time constants and can be used as generic arguments.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    static constexpr uint N = 3;
    vector<uint, N> v = { 5, 6, 7 };
    outputBuffer[0] = v[0] + v[1] + v[2];
}

// CHECK: 18
