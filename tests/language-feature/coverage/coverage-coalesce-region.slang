// Verifies that line coverage emits one counter per straight-line
// region rather than one per statement. Emitted shader size scales with
// the number of probe sequences, so collapsing a run of statements that
// provably execute together is what keeps instrumented code small.
//
// Region boundaries fall out of the IR's basic-block structure: the
// if/else arms below each begin a new block, so they cannot be merged
// with the statements before or after them.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage
//TEST:SIMPLE(filecheck=ONEPROBE):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // These three statements plus the `if` statement itself are one
    // straight-line region and share a single counter.
    uint a = tid.x;
    uint b = a + 1;
    uint c = b + 2;
    if (c > 3)
    {
        // Separate block, so a separate region.
        c += 1;
        c += 2;
    }
    else
    {
        c += 3;
    }
    outputBuffer[0] = c;
}

// Exactly four regions: the entry run, the then-arm, the else-arm, and
// the statement after the merge. Slots are assigned in traversal order.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}

// ...and no fifth. If coalescing regresses to one counter per
// statement, slot 4 appears and this fails.
//CHECK-NOT: _slang_coverage{{.*int\(4\)}}

// Slot count alone does not pin the point of this pass. `outCounterCount` is
// computed from the slot assignment, independently of which marker emits a
// probe, so a regression that shared slots correctly but left every marker
// emitting would produce N redundant atomics into one slot while the counter
// count, every entry's counterIndex and the LCOV output stayed byte-identical.
// The region has to emit exactly ONE runtime probe, not just occupy one slot.
//ONEPROBE-COUNT-1: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//ONEPROBE-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
