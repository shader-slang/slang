//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type

// This is the motivating issue shape: indexing an `RWStructuredBuffer` produces a writable value,
// and the default `inout` receiver writes a method's field mutation back to that value.

struct Counter
{
    int value;

    void increment()
    {
        value++;
    }
};

//TEST_INPUT:ubuffer(data=[41], stride=4):out,name=counters
RWStructuredBuffer<Counter> counters;

[numthreads(1, 1, 1)]
void computeMain()
{
    counters[0].increment();
    // CHECK: 42
}
