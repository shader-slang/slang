//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify an infinitely growing recursive field type fails cleanly during
// lowering instead of recursing indefinitely through ctor/layout paths.

struct LoopField<each T>
{
    LoopField<T, int> next;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    LoopField<int> value = {};
}
//CHECK: maximum type nesting level exceeded
