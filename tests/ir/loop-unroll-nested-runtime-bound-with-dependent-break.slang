//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The inner `[ForceUnroll]` loop has a runtime header bound (`j < n`) that this pass cannot
// fold, *and* an early `if (j >= i) break` that depends on the enclosing `[ForceUnroll]`
// loop's index. The runtime bound does not statically bound the peeling, so it must not
// override deferral: once the outer loop unrolls, `i` becomes a literal here (its `i = 0; ++i`
// start and step are compile-time). This must not hang peeling to the iteration limit or
// report a spurious E40020 — it must compile. (With a
// runtime `n` the inner loop need not fully vanish, so this test only pins that compilation
// succeeds without an unroll error, not that all loops are eliminated.)

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform int n)
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0; j < n; ++j)
        {
            if (j >= i)
                break;
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// If this loop were misclassified the compile would fail with E40020 and emit no function, so
// the presence of the emitted entry point is the pass signal.
// CHECK: void computeMain
