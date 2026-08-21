// unit-test-ir-dce.cpp
//
// Tests for global dead-code elimination (`eliminateDeadCode` in
// `slang-ir-dce.cpp`), exercised directly on hand-built IR.
//
// Consider what it takes to test the following claim: "global DCE removes a
// top-level function that nothing references and that carries no `[KeepAlive]`
// decoration." Expressing that through a `.slang` end-to-end test is awkward,
// because the frontend arranges for entry points and their transitive callees
// to stay live, which is precisely the condition the pass discriminates on.
// Building the module by hand removes the scaffolding: place exactly two
// functions, decorate one, run the pass, and assert which one survived.

#include "slang/slang-ir-dce.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// An unreferenced function without `[KeepAlive]` is removed, while an
// unreferenced function with it is retained.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedFunction)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunction("keptFunc", /* keepAlive: */ true);
    builder.addVoidFunction("deadFunc", /* keepAlive: */ false);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 2);

    const bool changed = eliminateDeadCode(builder.getModule());
    SLANG_CHECK(changed);

    // Assert on identity rather than a count, so a failure names the function
    // that behaved unexpectedly.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "keptFunc");
}

// Running the pass on a module that has nothing to remove reports that it made
// no change. Passes report `changed` so a fixpoint loop can terminate, so a
// pass that always claims to have changed something is a real defect.
SLANG_UNIT_TEST(irDeadCodeEliminationReportsNoChangeWhenNothingIsDead)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunction("liveFunc", /* keepAlive: */ true);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 1);

    const bool changed = eliminateDeadCode(builder.getModule());
    SLANG_CHECK(!changed);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 1);
}

// Every unreferenced, undecorated function is removed, not merely the first one
// encountered. A pass that stops after its first removal, or that invalidates
// its own iteration when it edits the instruction list, would pass a
// single-function test and fail this one.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesAllDeadFunctions)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunction("dead0", /* keepAlive: */ false);
    builder.addVoidFunction("kept", /* keepAlive: */ true);
    builder.addVoidFunction("dead1", /* keepAlive: */ false);
    builder.addVoidFunction("dead2", /* keepAlive: */ false);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 4);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "kept");
}

// The pass is idempotent: running it a second time on its own output finds
// nothing further to remove.
SLANG_UNIT_TEST(irDeadCodeEliminationIsIdempotent)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunction("kept", /* keepAlive: */ true);
    builder.addVoidFunction("dead", /* keepAlive: */ false);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    // Pin *which* function survived, not merely how many. Comparing the second run
    // against the first run's own output would hold even if the first run had wrongly
    // removed `kept` as well: the count would be 0 both times, the second run would
    // report no change, and every assertion would pass.
    List<String> afterFirstRun = builder.getFunctionNames();
    SLANG_CHECK_ABORT(afterFirstRun.getCount() == 1);
    SLANG_CHECK(afterFirstRun[0] == "kept");

    SLANG_CHECK(!eliminateDeadCode(builder.getModule()));

    List<String> afterSecondRun = builder.getFunctionNames();
    SLANG_CHECK_ABORT(afterSecondRun.getCount() == 1);
    SLANG_CHECK(afterSecondRun[0] == "kept");
}

// A function with no decoration of its own survives when a live function calls
// it. This is the other half of the pass's contract: the tests above cover what
// gets removed, while this covers what reachability keeps alive. A regression
// that stopped following call operands when marking live instructions would
// leave every test above green and fail only this one.
SLANG_UNIT_TEST(irDeadCodeEliminationKeepsFunctionReachableFromLiveRoot)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* callee = builder.addVoidFunction("calleeFunc", /* keepAlive: */ false);
    builder.addVoidFunctionCalling("rootFunc", /* keepAlive: */ true, callee);
    builder.addVoidFunction("deadFunc", /* keepAlive: */ false);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 3);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    // `calleeFunc` carries no decoration and is kept only by the reference from
    // `rootFunc`, while `deadFunc` differs solely in not being referenced.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 2);
    SLANG_CHECK(names.contains("rootFunc"));
    SLANG_CHECK(names.contains("calleeFunc"));
    SLANG_CHECK(!names.contains("deadFunc"));
}

// Reachability is transitive: a function reached only through an intermediate
// callee is still kept. A pass that marked only the direct operands of a live
// root would keep the intermediate but drop the function beyond it.
SLANG_UNIT_TEST(irDeadCodeEliminationKeepsTransitivelyReachableFunction)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* leaf = builder.addVoidFunction("leafFunc", /* keepAlive: */ false);
    IRFunc* middle = builder.addVoidFunctionCalling("middleFunc", /* keepAlive: */ false, leaf);
    builder.addVoidFunctionCalling("rootFunc", /* keepAlive: */ true, middle);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 3);

    // Note the inverted assertion relative to the test above, which expects a change:
    // there, `deadFunc` is genuinely dead and gets removed. Here all three functions are
    // live, nothing is removed, and the pass reports no change -- its return value
    // accumulates only from removals.
    SLANG_CHECK(!eliminateDeadCode(builder.getModule()));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 3);
    SLANG_CHECK(names.contains("leafFunc"));
}

// The tests above all turn on reachability. This pair pins an *option* instead:
// `keepGlobalParamsAlive` defaults to true, so an unreferenced global parameter
// survives an ordinary run. Shader parameters rely on that to reach reflection even
// when no code reads them, and nothing else here would notice if the default were
// flipped.
SLANG_UNIT_TEST(irDeadCodeEliminationKeepsUnreferencedGlobalParamByDefault)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addGlobalParam("unusedParam");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_GlobalParam) == 1);

    // Nothing is removed, so the pass reports no change -- asserted here for the same
    // reason as elsewhere in this file, since a pass that always claims to have changed
    // something would never let a fixpoint loop terminate.
    SLANG_CHECK(!eliminateDeadCode(builder.getModule()));

    SLANG_CHECK(builder.countGlobalInsts(kIROp_GlobalParam) == 1);
}

// ...and with the flag off the same parameter is removed. Paired with the test above,
// this pins the flag itself rather than only the value it happens to default to: a
// change that ignored the option would fail exactly one of the two.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedGlobalParamWhenNotKeptAlive)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addGlobalParam("unusedParam");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_GlobalParam) == 1);

    IRDeadCodeEliminationOptions options;
    options.keepGlobalParamsAlive = false;
    SLANG_CHECK(eliminateDeadCode(builder.getModule(), options));

    SLANG_CHECK(builder.countGlobalInsts(kIROp_GlobalParam) == 0);
}

// `keepExportsAlive` is off by default, so an unreferenced `[Export]` function is
// removed on an ordinary run and kept when the flag is set. Linkage decorations are
// what a separately-compiled module needs to survive, so the two settings answer
// genuinely different questions and neither is universally right.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedExportByDefault)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addExportedVoidFunction("exportedFunc");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_Func) == 1);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 0);
}

SLANG_UNIT_TEST(irDeadCodeEliminationKeepsUnreferencedExportWhenAsked)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addExportedVoidFunction("exportedFunc");

    IRDeadCodeEliminationOptions options;
    options.keepExportsAlive = true;
    SLANG_CHECK(!eliminateDeadCode(builder.getModule(), options));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "exportedFunc");
}

// The same pairing for `keepLayoutsAlive`. Layout decorations are what reflection
// reads, so a pass run with the flag set must not drop the instructions carrying them.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedLayoutByDefault)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunctionWithLayout("laidOutFunc");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_Func) == 1);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 0);
}

SLANG_UNIT_TEST(irDeadCodeEliminationKeepsUnreferencedLayoutWhenAsked)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    builder.addVoidFunctionWithLayout("laidOutFunc");

    IRDeadCodeEliminationOptions options;
    options.keepLayoutsAlive = true;
    SLANG_CHECK(!eliminateDeadCode(builder.getModule(), options));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "laidOutFunc");
}

// A weak operand must not hold its referent alive. This is the mechanism behind
// `isWeakReferenceOperand`, and it is the exact inverse of the reachability tests
// above: an ordinary call operand keeps a callee, a `WeakUse` operand must not.
// Without a test, a change that stopped consulting `isWeakReferenceOperand` would
// look like "DCE got more conservative" rather than a defect.
SLANG_UNIT_TEST(irDeadCodeEliminationDoesNotKeepAWeaklyReferencedFunction)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* target = builder.addVoidFunction("weaklyReferencedFunc", /* keepAlive: */ false);
    builder.addVoidFunction("rootFunc", /* keepAlive: */ true);
    builder.addLiveWeakUseOf(target);
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_Func) == 2);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    // The `WeakUse` has to survive, or the test proves nothing: `weaklyReferencedFunc`
    // would then be unreferenced by anything live and removed for the ordinary reason.
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_WeakUse) == 1);

    // So this assertion turns entirely on the operand being classified weak. Drop that
    // classification and `weaklyReferencedFunc` is marked live and survives.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "rootFunc");
}

// Removing an unused block parameter is the one path that makes the pass iterate its
// work list more than once (`phiRemoved` in slang-ir-dce.cpp): the parameter goes, and
// the branch argument feeding it has to go with it. A regression in that rerun loop --
// or in its termination -- would leave every other test in this file green.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesAnUnusedBlockParameter)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* func = builder.addVoidFunctionWithUnusedBlockParam("rootFunc", /* keepAlive: */ true);

    // `IRInstList` has no count, so tally by walking.
    auto countBlockParams = [](IRFunc* f)
    {
        Int total = 0;
        for (IRBlock* block : f->getBlocks())
            for (IRParam* param : block->getParams())
            {
                SLANG_UNUSED(param);
                total++;
            }
        return total;
    };

    SLANG_CHECK_ABORT(countBlockParams(func) == 1);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    SLANG_CHECK(countBlockParams(func) == 0);

    // The function itself is a live root, so it must survive its parameter's removal.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "rootFunc");
}

// The `IRInst*` overload runs the pass rooted at one instruction rather than the whole
// module. Nothing else here covers it, so a change that broke rooted runs -- or that
// let one escape its root and edit the rest of the module -- would go unnoticed.
SLANG_UNIT_TEST(irDeadCodeEliminationOnASingleRootLeavesTheRestOfTheModuleAlone)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* rooted =
        builder.addVoidFunctionWithUnusedBlockParam("rootedFunc", /* keepAlive: */ true);
    builder.addVoidFunction("untouchedDeadFunc", /* keepAlive: */ false);

    SLANG_CHECK(eliminateDeadCode(rooted));

    // The dead function is outside the root, so a rooted run must not have removed it.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 2);
    SLANG_CHECK(names.contains("untouchedDeadFunc"));
}

// `trimOptimizableTypes` is a separate entry point from `eliminateDeadCode`, and drops
// unreferenced fields from a struct marked `[OptimizableType]`.
SLANG_UNIT_TEST(irTrimOptimizableTypesRemovesAnUnusedField)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    auto countFields = [](IRStructType* t)
    {
        Int total = 0;
        for (IRStructField* field : t->getFields())
        {
            SLANG_UNUSED(field);
            total++;
        }
        return total;
    };

    IRStructType* structType = builder.addOptimizableStructWithUnusedField("Optimizable");
    SLANG_CHECK_ABORT(countFields(structType) == 1);

    SLANG_CHECK(trimOptimizableTypes(builder.getModule()));
    SLANG_CHECK(countFields(structType) == 0);
}

// `useFastAnalysis` selects a cheaper side-effect analysis. For module shapes like
// these -- functions with no side effects to analyse -- it is expected to reach the
// same answer as the precise path, and this pins that: same fixture, same outcome,
// flag on.
//
// It deliberately does not claim more. Distinguishing the two paths properly needs a
// fixture whose purity determination actually differs between them, which is a
// different kind of fixture from anything `IRFixtureBuilder` builds today. This test
// is the honest half of that: it catches a fast path that starts disagreeing on the
// simple shapes, and says nothing about the shapes where disagreement is the point.
SLANG_UNIT_TEST(irDeadCodeEliminationFastAnalysisAgreesOnSideEffectFreeFunctions)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

    IRFunc* callee = builder.addVoidFunction("calleeFunc", /* keepAlive: */ false);
    builder.addVoidFunctionCalling("rootFunc", /* keepAlive: */ true, callee);
    builder.addVoidFunction("deadFunc", /* keepAlive: */ false);

    IRDeadCodeEliminationOptions options;
    options.useFastAnalysis = true;
    SLANG_CHECK(eliminateDeadCode(builder.getModule(), options));

    // Identical to what irDeadCodeEliminationKeepsFunctionReachableFromLiveRoot asserts
    // with the flag off.
    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 2);
    SLANG_CHECK(names.contains("rootFunc"));
    SLANG_CHECK(names.contains("calleeFunc"));
    SLANG_CHECK(!names.contains("deadFunc"));
}
