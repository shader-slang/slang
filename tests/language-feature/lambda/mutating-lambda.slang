//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct Counter
{
    uint value;

    [mutating]
    uint next()
    {
        value++;
        return value;
    }
}

uint invoke<TFunc : IMutatingFunc<uint>>(inout TFunc function)
{
    return function();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint capturedScalar = 3;
    var scalarLambda = ()! =>
    {
        capturedScalar++;
        return capturedScalar;
    };

    outputBuffer[0] = invoke(scalarLambda);
    // CHECK: 4
    outputBuffer[1] = scalarLambda();
    // CHECK: 5
    outputBuffer[2] = capturedScalar;
    // CHECK: 3

    Counter capturedCounter = {10};
    var counterLambda = ()! => capturedCounter.next();
    outputBuffer[3] = counterLambda();
    // CHECK: 11
    outputBuffer[4] = invoke(counterLambda);
    // CHECK: 12
    outputBuffer[5] = capturedCounter.value;
    // CHECK: 10
}
