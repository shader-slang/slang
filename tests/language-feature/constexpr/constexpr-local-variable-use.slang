// Verify that a non-static local `constexpr` variable (which falls back to
// `const` with a warning) is usable as a compile-time constant, e.g., as an
// array dimension or vector element count.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-31227
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int N = 4;
    vector<uint, N> v = { 1, 2, 3, 4 };
    uint sum = 0;
    for (int i = 0; i < N; i++)
        sum += v[i];
    outputBuffer[0] = sum;
}

// CHECK: 10
