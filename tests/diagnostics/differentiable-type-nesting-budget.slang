//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Verify recursive differentiable-type registration reports the shared
// nesting diagnostic instead of recursing indefinitely.

struct LoopField<each T>
{
    LoopField<T, int> next;
}

[Differentiable]
void recursiveParam(LoopField<int> value)
//CHECK:            ^^^^^^^^^ maximum type nesting level exceeded
{
}
