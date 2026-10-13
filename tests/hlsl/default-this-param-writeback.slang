//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type

// This is the motivating issue shape: indexing an `RWStructuredBuffer` produces a writable value,
// and lowering writes the method's field mutation back through the default `inout` receiver.

struct Counter
{
    int value;

    void increment()
    {
        value++;
    }
};

//TEST_INPUT: set counters = ubuffer(data=[41], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<Counter> counters;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    counters[0].increment();
    outputBuffer[0] = counters[0].value;
}

// CHECK: type: int32_t
// CHECK-NEXT: 42
