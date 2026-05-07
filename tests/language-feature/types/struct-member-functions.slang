// struct-member-functions.slang

// Test member functions (immutable this by default) and [mutating] methods.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Accumulator
{
    int value;

    // Immutable method: cannot modify `this`
    int get()
    {
        return value;
    }

    // Immutable method: returns computed result without modifying state
    int doubled()
    {
        return value * 2;
    }

    // Mutating method: can modify `this`
    [mutating]
    void add(int amount)
    {
        value = value + amount;
    }

    // Mutating method: resets the value
    [mutating]
    void reset()
    {
        value = 0;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Accumulator acc;
    acc.value = 10;

    // Immutable method returns value without modification
    // CHECK: 10
    outputBuffer[0] = acc.get();

    // Immutable method returns computed value
    // CHECK: 20
    outputBuffer[1] = acc.doubled();

    // Mutating method modifies the struct
    acc.add(5);

    // CHECK: 15
    outputBuffer[2] = acc.get();

    // Mutating reset
    acc.reset();
    acc.add(42);

    // CHECK: 42
    outputBuffer[3] = acc.get();
}
