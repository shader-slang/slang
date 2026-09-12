// Covers the two exit-analysis outcomes that the other coalescing tests
// leave out, both of which fail silently if they regress.
//
// 1. A call to ordinary builtin math must NOT split a region. Those bodies
//    reach this pass unspecialized, so if the analysis reported them
//    unresolvable, or failed to count their exits as normal, every numeric
//    call would split its region and coalescing would be defeated for
//    essentially all real shader code -- with the counter buffer still
//    looking healthy. `max`, `sqrt` and `clamp` are used because they are
//    verified not to split; note that not every builtin qualifies, since
//    a generic whose body dispatches through a witness table (`lerp`,
//    `dot`) is unresolvable at this point and splits conservatively.
//
// 2. A function that never returns normally MUST split. `spin` has no
//    reachable return at all, which is the one way to reach the
//    "no normal exit anywhere" branch: a helper with an unconditional
//    `discard` does not, because lowering still appends an implicit
//    `return`, so it splits through the abandons-the-invocation path
//    instead (covered by coverage-coalesce-exit-split.slang).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// No reachable return: the region containing a call to this must split.
float spin(float x)
{
    for (;;)
    {
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // One region: the builtin calls do not break it up.
    float a = (float)tid.x;
    float b = max(a, 1.0f);
    float c = sqrt(b);
    float d = clamp(c, 0.0f, 1.0f);
    // Second region starts here, because `spin` never returns.
    float e = spin(d);
    outputBuffer[0] = e;
}

// `spin`'s own body, then the two regions of `computeMain`.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}

// Exactly three. A fourth slot means a builtin call started splitting the
// first region; slot 2 going missing means `spin` stopped splitting.
//CHECK-NOT: _slang_coverage{{.*int\(3\)}}
