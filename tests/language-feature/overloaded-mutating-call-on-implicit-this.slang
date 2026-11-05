//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// An overloaded `[mutating]` or `[__ref]` method called through the implicit `this` of a method
// whose `this` is `in` writes to that method's local copy of `this`, so the later read in `get`
// sees the write. A copy of an immutable receiver argument is passed only when semantic checking
// decided so for an HLSL call site (issue #13964), which does not apply to this Slang code.

struct MutatingCounter
{
    int x;

    [mutating]
    void inc() { x++; }

    [mutating]
    void inc(int delta) { x += delta; }

    int get()
    {
        inc();
        return x;
    }
};

struct RefCounter
{
    int x;

    [__ref]
    void inc() { x++; }

    [__ref]
    void inc(int delta) { x += delta; }

    int get()
    {
        inc();
        return x;
    }
};

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    MutatingCounter mutatingCounter;
    mutatingCounter.x = 1;
    // CHECK: 207
    outputBuffer[0] = mutatingCounter.get() * 100 + 7;

    RefCounter refCounter;
    refCounter.x = 1;
    // CHECK: 207
    outputBuffer[1] = refCounter.get() * 100 + 7;
}
