// unit-test-coverage-coalescing.cpp
//
// Tests for the coverage counter-coalescing analysis
// (`assignCoverageCounterSlots` in `slang-ir-coverage-instrument.cpp`),
// exercised directly on hand-built IR.
//
// Coalescing merges the line-coverage markers of one straight-line region onto
// a single counter and a single runtime probe. Its correctness rests on four
// properties. Driving the analysis on hand-built IR lets each be asserted
// directly on the slot assignment, which an end-to-end `.slang` test cannot do
// cleanly:
//
//   1. A `GenericAsm` terminator counts as a normal exit, so a call to an
//      intrinsic-backed helper does not split a region.
//   2. The probe sits at the *last* marker of a region.
//   3. A mid-region `Abort` splits the region.
//   4. Mutual recursion is broken conservatively and order-independently: an
//      optimistic cycle-break would let one partner be memoized as "returns
//      normally" depending on which one the traversal reached first.
//
// Property 2 is the sharpest case for a direct test: first- and last-marker
// placement emit the same *number* of probes and produce identical coverage
// totals, so a count- or result-based end-to-end test cannot distinguish them;
// the per-marker `outEmitsProbe` output is the precise, emission-independent
// way to observe the placement. The other three are reachable in principle
// from source (a builtin like `sqrt`, whose CPU body is a bare `GenericAsm`,
// lowers through one; `abort()` lowers to `Abort`; recursion survives to this
// pass), but pinning them end-to-end would mean building whole instrumentable
// modules and asserting on emitted probe instructions, which cannot isolate the
// specific classification the way a direct assertion on the slot assignment
// does. (`dot`/`lerp` are the opposite case — they dispatch through a witness
// table unresolved at this point and split conservatively, so they cannot
// stand in for the no-split property 1.)

#include "slang/slang-ir-coverage-instrument.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Property 2: a straight-line run coalesces onto one counter with the single
// probe on the *last* marker (see the file header for why only a direct
// `outEmitsProbe` assertion observes this).
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

    SLANG_CHECK(counterCount == 1);
    SLANG_CHECK(slots[0] == slots[1]);
    SLANG_CHECK(slots[1] == slots[2]);
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

    SLANG_CHECK(counterCount == 2);
    SLANG_CHECK(slots[0] != slots[1]);
    SLANG_CHECK(emitsProbe[0]);
    SLANG_CHECK(emitsProbe[1]);
}

// Property 4: mutual recursion is broken conservatively and order-independently.
// `addMutuallyRecursiveFunctions` documents why the pair is asymmetric; the test
// analyzes both caller regions in ONE `assignCoverageCounterSlots` pass (so the
// may-not-return cache carries between them), then again with the order reversed.
// Requiring both regions to split in both orders is what pins order-independence:
// an unsound optimistic cycle-break coalesces the `b` region only when `a` is
// resolved first.
SLANG_UNIT_TEST(coverageCoalescingSplitsAtMutuallyRecursiveCallEitherOrder)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* a = nullptr;
    IRFunc* b = nullptr;
    builder.addMutuallyRecursiveFunctions("a", "b", a, b);

    List<IRInst*> callsA = builder.addMarkerRunAroundCall("callsA", a);
    List<IRInst*> callsB = builder.addMarkerRunAroundCall("callsB", b);
    SLANG_CHECK_ABORT(callsA.getCount() == 2);
    SLANG_CHECK_ABORT(callsB.getCount() == 2);

    auto checkBothRegionsSplit = [&](List<IRInst*> const& markers)
    {
        List<UInt> slots;
        List<bool> emitsProbe;
        UInt counterCount = 0;
        assignCoverageCounterSlots(markers, slots, emitsProbe, counterCount);
        SLANG_CHECK(counterCount == 4);
        SLANG_CHECK(slots[0] != slots[1]);
        SLANG_CHECK(slots[2] != slots[3]);
    };

    List<IRInst*> aThenB;
    aThenB.addRange(callsA);
    aThenB.addRange(callsB);
    checkBothRegionsSplit(aThenB);

    List<IRInst*> bThenA;
    bThenA.addRange(callsB);
    bThenA.addRange(callsA);
    checkBothRegionsSplit(bThenA);
}

// A function-entry marker between two line markers is not an
// `IncrementCoverageCounter`, so it takes its own slot and neither opens nor
// breaks a line run — the two line markers around it still coalesce across it.
// One fixture pins both halves. (A branch marker takes the identical
// non-`IncrementCoverageCounter` path; only the function-marker case is built.)
SLANG_UNIT_TEST(coverageCoalescingGivesFunctionMarkerADedicatedSlotWithoutBreakingTheRun)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    // markers[0] = line, markers[1] = function-entry marker, markers[2] = line.
    List<IRInst*> markers = builder.addLineMarkersAroundFunctionMarker("funcMarkerRun");
    SLANG_CHECK_ABORT(markers.getCount() == 3);

    List<UInt> slots;
    List<bool> emitsProbe;
    UInt counterCount = 0;
    assignCoverageCounterSlots(markers, slots, emitsProbe, counterCount);

    SLANG_CHECK(counterCount == 2);
    SLANG_CHECK(slots[0] == slots[2]);
    SLANG_CHECK(slots[0] != slots[1]);
    SLANG_CHECK(!emitsProbe[0]);
    SLANG_CHECK(emitsProbe[1]);
    SLANG_CHECK(emitsProbe[2]);
}
