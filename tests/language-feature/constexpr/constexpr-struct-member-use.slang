// Test that a `constexpr` struct member (which falls back to `const` with a
// warning) is usable as a compile-time constant, e.g., as an array size.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-31227
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

RWStructuredBuffer<uint> outputBuffer;

struct Config
{
    static constexpr uint kSize = 4;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    vector<uint, Config::kSize> v = { 1, 2, 3, 4 };
    uint sum = 0;
    for (uint i = 0; i < Config::kSize; i++)
        sum += v[i];
    outputBuffer[0] = sum;
}

// CHECK: 10
