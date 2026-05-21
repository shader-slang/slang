// Test that `constexpr` variables can be used as arguments to user-defined
// generic types, exercising the constant-folding path in the semantic checker.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

static constexpr uint N = 2;

struct Pair<let Count : uint>
{
    uint data[Count];
};

[numthreads(1, 1, 1)]
void computeMain()
{
    Pair<N> p;
    p.data[0] = 100;
    p.data[1] = 200;
    outputBuffer[0] = p.data[0] + p.data[1];
}

// CHECK: 300
