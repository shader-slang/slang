//TEST:SIMPLE(filecheck=CHECK): -entry computeMain -profile cs_5_0 -target hlsl

// A generic array extent `N / M` is a symbolic `BuiltinOperationIntVal(Div)`. Specializing it
// with `M == 0` must fold through `BuiltinOperationIntVal::tryFoldImpl`'s divide-by-zero guard
// and emit a diagnostic, rather than crashing during constant folding.

RWStructuredBuffer<int> outputBuffer;

// CHECK: divide by zero
int f<int N, int M>()
{
    int a[N / M];
    return a.getCount();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = f<10, 0>();
}
