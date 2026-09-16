//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for the `DeferBufferLoadContext::calleeSideEffectCache` invariant added in
// PR #13012 (fixes #13010): the cache must never let load/store redundancy removal forward a
// value across a call to a function that writes the same address. `deferBufferLoadInFunc` calls
// `removeRedundancyInFunc(func, false, &calleeSideEffectCache)` on every function this pass
// processes, so a stale "no side effect" entry there would let `canInstHaveSideEffectAtAddress`
// (slang-ir-util.cpp) treat `mutate` below as pure and eliminate the second load of
// `outputBuffer[0]`, reusing the pre-call store's value (1) instead of re-reading the post-call
// value (42). `mutate` is `[noinline]` so it survives as a real `IRCall` at redundancy-removal
// time instead of being inlined away before this pass runs.

RWStructuredBuffer<uint> outputBuffer;

[noinline]
void mutate()
{
    outputBuffer[0] = 42;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 1;
    mutate();
    // A correct compile must re-read outputBuffer[0] here and see 42, not reuse the store above.
    uint v = outputBuffer[0];
    outputBuffer[1] = v;
}

//CHECK: 42
//CHECK-NEXT: 42
