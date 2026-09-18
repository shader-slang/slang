// A function-local generic struct that is missing an interface requirement must produce
// a clean "missing interface member" diagnostic (E38100) during semantic checking,
// rather than reaching code generation.
// Regression test for https://github.com/shader-slang/slang/issues/13092.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

interface IOp
{
    int apply<T>(T value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    struct Bad<V> : IOp
    {
        V val;
    };
    Bad<int> b;
    b.val = 1;
    outputBuffer[0] = 0;
}
//CHECK: E38100
//CHECK: does not provide required interface member 'apply'
