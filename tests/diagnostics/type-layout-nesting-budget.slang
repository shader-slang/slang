//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify recursive type layout creation reports the shared nesting
// diagnostic on a parameter-block layout path.

struct LoopField<each T>
{
    LoopField<T, int> next;
//CHECK:              ^^^^ maximum type nesting level exceeded
}

ParameterBlock<LoopField<int>> gParams;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
