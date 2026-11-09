// A function that returns on one path and diverges on another must split its
// caller's region, even though a normal return is present in its body.
//
// This is the case a "does the body contain a return?" check gets wrong, and
// it fails in the dangerous direction. `maybeDiverge` lowers to an `ifElse`
// whose else path is a reachable `return_val` and whose then path is an
// `unreachable`-terminated loop, with no discard, abort or unresolvable call
// anywhere. Treating the present return as proof of return would coalesce the
// caller across the call; when the loop is taken the probe never fires, and
// the statements before the call -- which did execute -- report zero hits.
//
// The analysis instead asks whether every block reachable from the entry can
// still reach a normal exit. The loop block can reach none, so the call splits.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Returns when `c` is false; never returns when it is true.
void maybeDiverge(bool c)
{
    if (c)
    {
        for (;;)
        {
        }
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // One region up to and including the call...
    float a = (float)tid.x;
    maybeDiverge(tid.x == 0);
    // ...and a second after it, because the callee may never come back.
    float b = a + 1.0f;
    outputBuffer[0] = b;
}

// Two regions in `maybeDiverge` (the `if` and its body), then the two in
// `computeMain` either side of the call.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}

// Exactly four. If the present-return check came back, `computeMain` would
// collapse to one region and slot 3 would disappear.
//CHECK-NOT: _slang_coverage{{.*int\(4\)}}
