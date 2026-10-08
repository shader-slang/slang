// Test that `constexpr` falls back to `const` semantics and the resulting
// variable can be used as a compile-time generic argument. Regression test
// for issue #10940 (static constexpr uint N used as vector size).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-31227

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

static constexpr uint N = 4;

[numthreads(1, 1, 1)]
void computeMain()
{
    vector<uint, N> v = { 1, 2, 3, 4 };

    uint sum = 0;
    for (uint i = 0; i < N; i++)
        sum += v[i];

    outputBuffer[0] = sum;
}

// CHECK: 10
