// generic-constructor-inference.slang

// Test that generic structs with constrained type parameters can be
// constructed using explicit generic arguments in the constructor call.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

struct Wrapper<T : IArithmetic>
{
    T val;

    __init(T v)
    {
        val = v;
    }

    T get() { return val; }
}

struct Pair<T : IArithmetic, U : IArithmetic>
{
    T first;
    U second;

    __init(T a, U b)
    {
        first = a;
        second = b;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Construct Wrapper with explicit type parameter int
    Wrapper<int> w1 = Wrapper<int>(3);
    // CHECK: 3
    output[0] = w1.get();

    // Construct Wrapper with explicit type parameter float
    Wrapper<float> w2 = Wrapper<float>(7.0f);
    // CHECK: 7
    output[1] = int(w2.get());

    // Construct Pair with explicit type parameters
    Pair<int, int> p = Pair<int, int>(10, 20);
    // CHECK: 30
    output[2] = p.first + p.second;
}
