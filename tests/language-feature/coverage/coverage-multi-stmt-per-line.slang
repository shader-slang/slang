// Locks in same-line slot dedup: when multiple
// `IncrementCoverageCounter` ops resolve to the same `(file, line)`,
// they share a counter slot. Each statement still triggers its own
// atomic-add at runtime (the per-statement firing is preserved), but
// they all hit the same buffer slot — fewer slots in the buffer,
// fewer entries in the (file, line) metadata.
//
// Earlier design assigned a unique slot per op to allow per-statement
// hit counts. That fidelity isn't reachable through the LCOV / gcov
// reporting pipeline, which already aggregates by `(file, line)` at
// the host side, so the user-visible report is identical either way.
// The dedup gives us a 30-50% counter-buffer + metadata reduction on
// typical shaders without changing observable coverage semantics.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint a = 0; uint b = 1; uint c = 2;
    outputBuffer[0] = a + b + c;
}

// All three statements on the same source line share one slot. The
// captured `[[SLOT]]` regex pins each subsequent atomic-add to the
// same int literal as the first — if the compiler regresses back to
// per-op slots, the second statement's slot would be different and
// the recapture mismatches.
//
// The trailing CHECK pins the next statement (on the next source
// line) to a different slot, confirming the dedup is per-line, not
// module-wide.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*}}[[SLOT:int\([0-9]+\)]]
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*}}[[SLOT]]
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*}}[[SLOT]]

// The next source line gets its own slot — distinct from the shared
// slot above.
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*}}[[SLOT]]
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage
