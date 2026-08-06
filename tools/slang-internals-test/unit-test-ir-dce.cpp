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

#include "internals-test-env.h"
#include "slang/slang-ir-dce.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// An unreferenced function without `[KeepAlive]` is removed, while an
// unreferenced function with it is retained.
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedFunction)
{
    InternalsTestEnv env(unitTestContext);
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
    InternalsTestEnv env(unitTestContext);
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
    InternalsTestEnv env(unitTestContext);
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
    InternalsTestEnv env(unitTestContext);
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
    InternalsTestEnv env(unitTestContext);
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
    InternalsTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSession());

    IRFunc* leaf = builder.addVoidFunction("leafFunc", /* keepAlive: */ false);
    IRFunc* middle = builder.addVoidFunctionCalling("middleFunc", /* keepAlive: */ false, leaf);
    builder.addVoidFunctionCalling("rootFunc", /* keepAlive: */ true, middle);
    SLANG_CHECK(builder.countGlobalInsts(kIROp_Func) == 3);

    SLANG_CHECK(!eliminateDeadCode(builder.getModule()));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 3);
    SLANG_CHECK(names.contains("leafFunc"));
}
