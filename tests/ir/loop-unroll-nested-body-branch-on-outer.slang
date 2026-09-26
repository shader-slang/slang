//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// A constant-bound inner `[ForceUnroll]` loop whose body — but not its exit test — references
// the outer loop's variable must still compile. Only branches that can exit the loop decide
// its trip count, so the `if (i != 0)` here does not make the inner loop depend on `i`. If it
// were misclassified as bound-dependent on the plain outer loop, compilation would fail with a
// spurious E40020 and emit no function; this test guards against classifying body control flow
// as a bound dependency. The pass signal is successful emission of the entry point.

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
            if (i != 0)
                sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// If misclassified, compilation would fail with E40020 and emit no function.
// CHECK: void computeMain
