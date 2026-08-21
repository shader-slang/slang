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
    IRFixtureBuilder builder(env.getSession());

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
    IRFixtureBuilder builder(env.getSession());

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
    IRFixtureBuilder builder(env.getSession());

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
    IRFixtureBuilder builder(env.getSession());

    builder.addVoidFunction("kept", /* keepAlive: */ true);
    builder.addVoidFunction("dead", /* keepAlive: */ false);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));
    const Int afterFirstRun = builder.countGlobalInsts(kIROp_Func);

    SLANG_CHECK(!eliminateDeadCode(builder.getModule()));
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == afterFirstRun);
}

// A function with no decoration of its own survives when a live function calls
// it. This is the other half of the pass's contract: the tests above cover what
// gets removed, while this covers what reachability keeps alive. A regression
// that stopped following call operands when marking live instructions would
// leave every test above green and fail only this one.
SLANG_UNIT_TEST(irDeadCodeEliminationKeepsFunctionReachableFromLiveRoot)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSession());

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
    IRFixtureBuilder builder(env.getSession());

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
    IRFixtureBuilder builder(env.getSession());

    builder.addGlobalParam("unusedParam");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_GlobalParam) == 1);

    eliminateDeadCode(builder.getModule());

    SLANG_CHECK(builder.countGlobalInsts(kIROp_GlobalParam) == 1);
}

// ...and with the flag off the same parameter is removed. Paired with the test above,
// this pins the flag itself rather than only the value it happens to default to: a
// change that ignored the option would fail exactly one of the two.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedGlobalParamWhenNotKeptAlive)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSession());

    builder.addGlobalParam("unusedParam");
    SLANG_CHECK_ABORT(builder.countGlobalInsts(kIROp_GlobalParam) == 1);

    IRDeadCodeEliminationOptions options;
    options.keepGlobalParamsAlive = false;
    SLANG_CHECK(eliminateDeadCode(builder.getModule(), options));

    SLANG_CHECK(builder.countGlobalInsts(kIROp_GlobalParam) == 0);
}
