//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Smoke test that the cache-threaded `doesCalleeHaveSideEffect` signature added in PR #13012
// (fixes #13010) still detects impurity correctly on a real, non-inlined call: `mutate` is
// `[noinline]` so it survives as a real `IRCall` at redundancy-removal time, and a correct
// compile must re-read `outputBuffer[0]` after the call rather than reuse the pre-call store's
// value. This does NOT exercise a *stale* cache entry -- `deferBufferLoadInFunc`'s cache starts
// empty and queries `mutate` exactly once, so it produces the same result cached or uncached, and
// would not catch a regression in the staleness contract itself (see
// `DeferBufferLoadContext::calleeSideEffectCache`'s doc comment in slang-ir-defer-buffer-load.cpp
// and the shared-cache contract in slang-ir-redundancy-removal.h). Constructing a genuinely
// discriminating test requires a callee whose purity changes *while* a shared cache from an
// earlier query is still live, which isn't reliably expressible from `.slang` source -- it
// depends on internal pass-iteration scheduling this test can't observe or control.

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
