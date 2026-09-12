// unit-test-coverage-coalescing.cpp
//
// Tests for the coverage counter-coalescing analysis
// (`assignCoverageCounterSlots` in `slang-ir-coverage-instrument.cpp`),
// exercised directly on hand-built IR.
//
// Coalescing merges the line-coverage markers of one straight-line region onto
// a single counter and a single runtime probe. Its correctness rests on four
// properties, and three of them cannot be written in `.slang` source at all:
//
//   1. A `GenericAsm` terminator counts as a normal exit, so a call to an
//      intrinsic-backed helper does not split a region. Every `.slang`
//      intrinsic that lowers to `GenericAsm` also contains a real `return`, so
//      source can never isolate the `GenericAsm` clause.
//   2. The probe sits at the *last* marker of a region. In a straight-line
//      block first- and last-marker placement produce the same probe *count*,
//      so an end-to-end test cannot tell a regression that moves it — only the
//      per-marker `outEmitsProbe` flag can.
//   3. A mid-region `Abort` splits the region. `Abort` is not reachable on its
//      own from source.
//   4. Mutual recursion is broken conservatively and order-independently: an
//      optimistic cycle-break would let one partner be memoized as "returns
//      normally" depending on traversal order.
//
// Building the IR by hand is what makes these directly assertable: place the
// markers and the abandoning instructions, run the analysis, and assert on the
// resulting slot assignment and probe placement.

#include "slang/slang-ir-coverage-instrument.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Property 2: a straight-line run of markers coalesces onto one counter, and
// the single probe is placed on the *last* marker of the run. Probe count alone
// cannot distinguish first- from last-marker placement, so this asserts the
// per-marker `outEmitsProbe` flags, which is the only observation that can.
SLANG_UNIT_TEST(coverageCoalescingPlacesProbeAtLastMarkerOfRegion)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    List<IRInst*> markers = builder.addStraightLineMarkerRun("straightLine", 3);
    SLANG_CHECK_ABORT(markers.getCount() == 3);

    List<UInt> slots;
    List<bool> emitsProbe;
    UInt counterCount = 0;
    assignCoverageCounterSlots(markers, slots, emitsProbe, counterCount);

    // All three markers share one counter.
    SLANG_CHECK(counterCount == 1);
    SLANG_CHECK(slots[0] == slots[1]);
    SLANG_CHECK(slots[1] == slots[2]);

    // Exactly the last marker emits the probe.
    SLANG_CHECK(!emitsProbe[0]);
    SLANG_CHECK(!emitsProbe[1]);
    SLANG_CHECK(emitsProbe[2]);
}

// Property 1: a `GenericAsm` terminator carries return semantics, so the exit
// analysis must classify a function that ends in one as returning normally, and
// a call to it must not split the surrounding region.
SLANG_UNIT_TEST(coverageCoalescingTreatsGenericAsmExitAsReturning)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* asmExit = builder.addFunctionEndingInGenericAsm("asmExit");
    List<IRInst*> markers = builder.addMarkerRunAroundCall("callsAsmExit", asmExit);
    SLANG_CHECK_ABORT(markers.getCount() == 2);

    List<UInt> slots;
    List<bool> emitsProbe;
    UInt counterCount = 0;
    assignCoverageCounterSlots(markers, slots, emitsProbe, counterCount);

    // The call did not split the run: both markers share one counter, probe on
    // the last.
    SLANG_CHECK(counterCount == 1);
    SLANG_CHECK(slots[0] == slots[1]);
    SLANG_CHECK(!emitsProbe[0]);
    SLANG_CHECK(emitsProbe[1]);
}

// Property 3: an `Abort` between two markers abandons the invocation, so the
// markers must not coalesce — each takes its own counter and its own probe.
SLANG_UNIT_TEST(coverageCoalescingSplitsRegionAtAbort)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    List<IRInst*> markers = builder.addMarkerRunSplitByAbort("aborter");
    SLANG_CHECK_ABORT(markers.getCount() == 2);

    List<UInt> slots;
    List<bool> emitsProbe;
    UInt counterCount = 0;
    assignCoverageCounterSlots(markers, slots, emitsProbe, counterCount);

    // Two separate regions: distinct counters, each its own probe.
    SLANG_CHECK(counterCount == 2);
    SLANG_CHECK(slots[0] != slots[1]);
    SLANG_CHECK(emitsProbe[0]);
    SLANG_CHECK(emitsProbe[1]);
}

// Property 4: mutual recursion is broken conservatively, and the answer does
// not depend on which partner the analysis reaches first. A pair `a`/`b` that
// only ever calls back and forth never reaches a non-recursive exit, so a call
// to either must split the surrounding region. The two calls below run through
// a fresh `CoverageFunctionExitAnalysis` each (one is created per
// `assignCoverageCounterSlots` call), so asserting both split is what pins the
// order-independence: an optimistic cycle-break would memoize one partner as
// returning-normally and let at least one order coalesce.
SLANG_UNIT_TEST(coverageCoalescingSplitsAtMutuallyRecursiveCallEitherOrder)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* a = nullptr;
    IRFunc* b = nullptr;
    builder.addMutuallyRecursiveFunctions("a", "b", a, b);

    List<IRInst*> aFirst = builder.addMarkerRunAroundCall("callsA", a);
    List<IRInst*> bFirst = builder.addMarkerRunAroundCall("callsB", b);
    SLANG_CHECK_ABORT(aFirst.getCount() == 2);
    SLANG_CHECK_ABORT(bFirst.getCount() == 2);

    // Reaching `a` first.
    {
        List<UInt> slots;
        List<bool> emitsProbe;
        UInt counterCount = 0;
        assignCoverageCounterSlots(aFirst, slots, emitsProbe, counterCount);
        SLANG_CHECK(counterCount == 2);
        SLANG_CHECK(slots[0] != slots[1]);
    }

    // Reaching `b` first, with a fresh analysis.
    {
        List<UInt> slots;
        List<bool> emitsProbe;
        UInt counterCount = 0;
        assignCoverageCounterSlots(bFirst, slots, emitsProbe, counterCount);
        SLANG_CHECK(counterCount == 2);
        SLANG_CHECK(slots[0] != slots[1]);
    }
}
