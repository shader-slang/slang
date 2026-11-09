//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// A `[ForceUnroll]` loop whose header test reads loop-carried state seeded from runtime values but
// which still terminates after a fixed number of iterations: `current` starts at a runtime `bool`
// but is overwritten from `next`, and `next` is pinned to `false` in the body, so after two
// iterations `current` is `false` and the loop exits regardless of the runtime seeds.
//
// The header test does not fold on the first peel (`current` is still the runtime seed), so the
// deferral pass leaves the loop; but the diagnosing pass peels it fully rather than bailing on that
// first non-folding peel, and the carried state converges, so the loop unrolls. This pins that the
// cheap deferral heuristic never turns a terminating loop into an unroll error: the bail is
// confined to the non-diagnosing pass.
//
// The pass signal is that compilation succeeds and the entry point is emitted: a `[ForceUnroll]`
// loop that failed to unroll would abort with E40020 and emit no function. (The unrolled iterations
// keep their runtime `if (current) ... else break` exit test, which the HLSL emitter renders as
// `for (;;) { ...; break; }` break-region scaffolding — that is not an iterating loop, so its
// presence is expected and not checked here.)

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform bool seed0, uniform bool seed1)
{
    int sum = 0;
    bool current = seed0;
    bool next = seed1;
    [ForceUnroll]
    while (current)
    {
        sum++;
        current = next;
        next = false;
    }
    outputBuffer[0] = sum;
}

// If the loop failed to unroll, compilation would abort with E40020 and emit no function.
// CHECK: void computeMain
