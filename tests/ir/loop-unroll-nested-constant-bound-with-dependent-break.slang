//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The inner `[ForceUnroll]` loop has a constant header bound (`j < 4`) *and* an early
// `if (j > i) break` whose condition depends on the outer index. The constant bound alone
// bounds the peeling, so the loop must be attempted now rather than rejected: the fast
// never-constant error is reported only when *every* exit depends on a non-unrollable
// enclosing loop and none is independent, and here the `j < 4` exit is independent. The outer
// loop is plain, so if this were misclassified it would wrongly report E40020 and emit no
// function. The pass signal is that compilation succeeds and the entry point is emitted.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0; j < 4; ++j)
        {
            if (j > i)
                break;
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// If misclassified, compilation would fail with E40020 and emit no function.
// CHECK: void computeMain
