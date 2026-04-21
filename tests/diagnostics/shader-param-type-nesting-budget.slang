//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify recursive shader-parameter semantic validation reports the shared
// nesting diagnostic instead of crashing on a recursive field walk.

struct LoopField<each T>
{
    LoopField<T, int> next;
}

struct Params
{
    LoopField<int> value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(Params p)
//CHECK:                ^ maximum type nesting level exceeded
{
}
