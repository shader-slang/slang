//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

// Regression test: `where T == S` should specialize for nominal types in a variadic pack.

struct S
{
    int value;
}

void add(inout int result, int value)
{
    result += value;
}

void acceptAll<each T>(inout int result, expand each T values) where T == S
{
    expand add(result, (each values).value);
}

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    S a = { 1 };
    S b = { 2 };
    int result = 0;
    acceptAll(result, a, b);

    // CHECK: 3
    outputBuffer[0] = result;
}
