// Regression test for thunk-based coverage instrumentation: each
// `IncrementCoverageCounter` op is lowered to a single
// `Call(__slang_coverage_hit, slot)` instead of expanding inline to
// a 3-inst atomic-add chain. The thunk is decorated `[ForceInline]`,
// so backends fold it back into a per-site atomic at emit time —
// i.e. the user-observable emit shape is preserved. This test
// verifies that property end-to-end on the CPU path: a generic
// helper instantiated for two types still produces one atomic-add
// per (specialization, source-line) in the emitted code, confirming
// the thunk transformation didn't drop any specialization's
// contribution.
//
// If `[ForceInline]` ever stops being honored on the synthesized
// thunk, this test would surface a `_slang_coverage_hit(` call in
// the output where an inline `_slang_atomic_add_u32` is expected,
// triggering a CHECK failure.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

// Second pass: a negative-only prefix scans the entire emitted
// output, catching a stray `_slang_coverage_hit` symbol regardless of
// whether it appears before, between, or after the atomic-add matches
// the first pass anchors to. (A trailing negative directive in the
// first pass only constrains text after its final positive match.)
//TEST:SIMPLE(filecheck=NOTHUNK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

T add_two<T : __BuiltinArithmeticType>(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int a = add_two<int>(1);
    float b = add_two<float>(2.0f);
    outputBuffer[0] = uint(a) + uint(b);
}

// Both specializations of `add_two<T>` keep their own atomic-add
// after the thunk gets inlined: one for `add_two<int>`, one for
// `add_two<float>`. Plus three for the statements in computeMain.
// Total: five inline atomic-adds against `__slang_coverage`.
//CHECK-COUNT-5: _slang_atomic_add_u32{{.*}}_slang_coverage
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage

// The thunk should be fully inlined, so no `_slang_coverage_hit`
// call survives in the CPU output. If this NOTHUNK-NOT triggers,
// either `[ForceInline]` regressed or the inlining pass stopped
// honoring it — both indicate the thunk is leaking into emit.
// Run as a separate filecheck pass so the directive scans the whole
// emitted output (a CHECK-NOT after CHECK-COUNT only covers text
// past the final positive match).
//NOTHUNK-NOT: _slang_coverage_hit{{[ \t]*\(}}
