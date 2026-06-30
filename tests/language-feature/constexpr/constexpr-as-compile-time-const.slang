// Test that `constexpr` variables are recognized as compile-time constants,
// matching the behaviour of `static const`. Regression test for issue #10940.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

static constexpr uint N = 4;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Use constexpr as a generic argument (the original repro).
    vector<uint, N> v = { 1, 2, 3, 4 };

    uint sum = 0;
    for (uint i = 0; i < N; i++)
        sum += v[i];

    outputBuffer[0] = sum;
}

// CHECK: 10
