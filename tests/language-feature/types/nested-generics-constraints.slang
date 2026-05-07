// nested-generics-constraints.slang

// Test nested generic types with constraints: a generic container holding
// elements that satisfy an interface, combined with constrained free functions.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface ICombine
{
    int combine(int a, int b);
}

struct Adder : ICombine
{
    int combine(int a, int b) { return a + b; }
}

struct Multiplier : ICombine
{
    int combine(int a, int b) { return a * b; }
}

struct Reducer<Op : ICombine>
{
    Op op;

    int reduce(int a, int b, int c)
    {
        int tmp = op.combine(a, b);
        return op.combine(tmp, c);
    }
}

int applyReducer<Op : ICombine>(Reducer<Op> r, int a, int b, int c)
{
    return r.reduce(a, b, c);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Reducer<Adder> addReducer;
    Reducer<Multiplier> mulReducer;

    // 3 + 4 + 5 = 12
    // CHECK: 12
    outputBuffer[0] = addReducer.reduce(3, 4, 5);
    // 3 * 4 * 5 = 60
    // CHECK: 60
    outputBuffer[1] = mulReducer.reduce(3, 4, 5);
    // Through generic function
    // CHECK: 12
    outputBuffer[2] = applyReducer(addReducer, 3, 4, 5);
    // CHECK: 60
    outputBuffer[3] = applyReducer(mulReducer, 3, 4, 5);
}
