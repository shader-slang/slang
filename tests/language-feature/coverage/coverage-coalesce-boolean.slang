// Coalescing and boolean recording are independent choices: boolean mode
// changes what a probe writes (a plain store of 1 instead of an atomic add),
// not where probes go. The design states placement is identical across the
// two rewrite forms, and nothing pinned that -- every other coalescing test
// uses the default counting mode.
//
// If coalescing were skipped under boolean mode, each statement would get its
// own slot and the emitted shader would carry the probe density the mode is
// often chosen to avoid.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage -trace-coverage-boolean
//TEST:SIMPLE(filecheck=ONESTORE):-target cpp -stage compute -entry computeMain -trace-coverage -trace-coverage-boolean

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // One straight-line region, exactly as under counting mode.
    uint a = tid.x;
    uint b = a + 1;
    uint c = b + 2;
    outputBuffer[0] = c;
}

// A single plain store, on slot 0, for the whole region.
//CHECK: _slang_coverage_0)[int(0)]) = 1

// No atomics at all: that is what boolean mode buys.
//CHECK-NOT: _slang_atomic_add

// And no second slot: coalescing is still in effect.
//CHECK-NOT: _slang_coverage_0)[int(1)]

// Same property on the boolean path: one store for the region, not one per
// statement sharing the slot. Checking only that slot 1 is absent would pass
// even if every marker in the region emitted its own store into slot 0.
//ONESTORE-COUNT-1: _slang_coverage_0)[int(0)]) = 1
//ONESTORE-NOT: _slang_coverage_0)[int(0)]) = 1
