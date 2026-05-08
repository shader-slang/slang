// Regression test for the unresolvable-source-location escape hatch
// in `assignSlot`.
//
// Counter-slot assignment dedupes by `(file, line)`. When
// `resolveHumaneLoc` cannot resolve the IR op's `sourceLoc`, every
// such op must get its own fresh slot — not collapse onto a single
// `":0"` dedup key. Without the escape hatch, all unresolvable-loc
// counter ops in a module would silently share one slot, merging
// unrelated execution paths in the reported coverage data.
//
// Trigger: `-obfuscate` rewrites source locs to point at a synthetic
// obfuscated source view that is opaque to the coverage pass's
// source-manager lookup, forcing `resolveHumaneLoc` to return false
// for every counter op in the module.
//
// Expected behavior with three statements on the same source line:
//
// - With `-obfuscate`: each op gets a distinct slot (escape hatch
//   fires) — three statements on one line yield three slots, one per
//   op, plus one for the assignment line. Slot literals 0, 1, 2, 3
//   appear in source order.
// - Without `-obfuscate` (compare with `coverage-multi-stmt-per-line`
//   in the same directory): the three statements share a single slot
//   via the dedup map.
//
// If the escape hatch ever regresses (e.g. the dedup map starts
// caching the unresolvable-loc empty key), the slot literals here
// would collapse onto a single value and the per-slot CHECK pattern
// would fail.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage -obfuscate

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int a = 1; int b = 2; int c = 3;
    outputBuffer[0] = uint(a + b + c);
}

// Each of the three statements on the same line gets its own slot
// because the obfuscated locs are unresolvable, so the dedup map
// never coalesces them. The fourth atomic-add belongs to the
// assignment line. Slot literals appear in source order.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*\[}}int(0)
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*\[}}int(1)
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*\[}}int(2)
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*\[}}int(3)
