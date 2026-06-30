// Test that a bare `constexpr` global (no `static`) is treated as a
// compile-time constant, not a shader parameter.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

constexpr uint N = 3;

[numthreads(1, 1, 1)]
void computeMain()
{
    vector<uint, N> v = { 10, 20, 30 };
    outputBuffer[0] = v[2];
}

// CHECK: 30
