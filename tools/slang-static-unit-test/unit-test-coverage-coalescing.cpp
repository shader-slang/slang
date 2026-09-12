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
// placement emit the same *number* of probes, so an end-to-end test cannot
// observe the difference at all — only the per-marker `outEmitsProbe` output
// can. The other three are reachable in principle from source (`sqrt`/`dot`
// lower through `GenericAsm`, `abort()` lowers to `Abort`, and recursion
// survives to this pass), but pinning them end-to-end would mean building whole
// instrumentable modules and asserting on emitted probe instructions, which
// cannot isolate the specific classification the way a direct assertion on the
// slot assignment does.

#include "slang/slang-ir-coverage-instrument.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Property 2: a straight-line run of markers coalesces onto one counter, with
// the single probe on the *last* marker. Probe count alone cannot tell first-
// from last-marker placement apart, so this asserts the per-marker
// `outEmitsProbe` flags — the only observation that can.
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

// Property 4: mutual recursion is broken conservatively, and the answer does
// not depend on which partner the analysis reaches first. The pair is
// asymmetric: `a` calls `b` then abandons via `Abort` (may-not-return on its
// own), while `b` calls `a` then returns (may-not-return only through the
// recursion into `a`). Both a call to `a` and a call to `b` must therefore
// split their region.
//
// The two regions are analyzed in one `assignCoverageCounterSlots` call, so the
// may-not-return cache built resolving the first region's call carries into the
// second. Reversing which region comes first is what pins order-independence:
// an unsound optimistic cycle-break memoizes `b` as returning-normally when `a`
// is resolved first, coalescing the `b` region in that order only.
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
    checkBothRegionsSplit(aThenB); // resolves `a` first

    List<IRInst*> bThenA;
    bThenA.addRange(callsB);
    bThenA.addRange(callsA);
    checkBothRegionsSplit(bThenA); // resolves `b` first
}
