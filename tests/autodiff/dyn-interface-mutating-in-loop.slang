// Backward autodiff of a loop that calls a `[mutating]` method on an `inout` dyn interface.
// The call's temporary has a type computed in the loop body; CFG normalization used to hoist
// it above that type, which made `legalizeDefUse` loop forever.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
dyn interface ICounter
{
    [mutating]
    float next();
};

// Returns 1, 2, 3, ...
struct Counter : ICounter
{
    uint state;

    [mutating]
    float next()
    {
        state++;
        return float(state);
    }
};

// Returns 2, 4, 6, ...
struct EvenCounter : ICounter
{
    uint state;

    [mutating]
    float next()
    {
        state += 2;
        return float(state);
    }
};

ICounter makeCounter(uint i)
{
    if (inputBuffer[i] == 0)
        return Counter(0);
    return EvenCounter(0);
}

[Differentiable]
float accumulate(float x, inout ICounter counter)
{
    float result = 0.0f;
    for (uint i = 0; i < 8; i++)
    {
        if (counter.next() > 2.5f)
            result += x;
    }
    return result;
}

[shader("compute"), numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ICounter counter = makeCounter(tid.x);
    var dp = diffPair(1.0f, 0.0f);
    bwd_diff(accumulate)(dp, counter, 1.0f);
    outputBuffer[tid.x] = dp.d;
}

// d/dx counts the values > 2.5 among the first 8: Counter 3..8 -> 6, EvenCounter 4..16 -> 7.
// CHECK: 6.0
// CHECK-NEXT: 7.0
