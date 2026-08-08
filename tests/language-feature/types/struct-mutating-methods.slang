// struct-mutating-methods.slang

// Test [mutating] methods on structs that modify `this`.
// Verifies value semantics: mutation through one copy doesn't affect another.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

struct Counter
{
    int count;

    [mutating]
    void increment()
    {
        count += 1;
    }

    [mutating]
    void add(int n)
    {
        count += n;
    }

    [mutating]
    void reset()
    {
        count = 0;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter c;
    c.count = 0;

    c.increment();
    c.increment();
    c.increment();
    // CHECK: 3
    outputBuffer[0] = c.count;

    // Value semantics: copy is independent
    Counter c2 = c;
    c2.add(10);
    // CHECK: 3
    outputBuffer[1] = c.count;
    // CHECK: 13
    outputBuffer[2] = c2.count;

    c2.reset();
    // CHECK: 0
    outputBuffer[3] = c2.count;
}
