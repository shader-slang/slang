// Pin how the peephole pass folds a block parameter whose incoming values include undefined
// values. A `LoadFromUninitializedMemory` behaves like LLVM's `freeze(undef)`: one instance yields
// one arbitrary but fixed value, and distinct instances may differ. A fold must therefore keep two
// properties. Correlation: for `p = phi(a, b)`, `p == a` holds on the `a` edge, so `p` may only
// become a fresh value when nothing else observes `a` or `b`. Loop stability: for
// `p = phi(entry: a, backedge: p)`, every iteration sees the same `a`, so the fold forwards `a`
// rather than creating a value in the loop header. Hand-built IR states each shape exactly, without
// depending on what SSA construction and earlier passes happen to produce.

#include "slang/slang-compiler.h"
#include "slang/slang-ir-peephole.h"
#include "slang/slang-ir-util.h"
#include "slang/slang-ir-validate.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

bool isValidModule(IRModule* module)
{
    DiagnosticSink sink;
    validateIRModule(module, &sink);
    return sink.getErrorCount() == 0;
}

Index countUninitializedLoads(IRBlock* block)
{
    Index count = 0;
    for (auto inst : block->getChildren())
    {
        if (inst->getOp() == kIROp_LoadFromUninitializedMemory)
            count++;
    }
    return count;
}

/// A function `T f(bool c)` shaped as a diamond, which the tests fill in:
///
///     entry(c): ...; ifElse(c, left, right, merge)
///     left:     ...; br merge(...)
///     right:    ...; br merge(...)
///     merge(params...): ...; return ...
///
/// `begin` builds the blocks and leaves `builder` inserting into the entry block. Each test then
/// emits into the blocks directly and terminates them with `finish*`.
struct Diamond
{
    IRFunc* func = nullptr;
    IRBlock* entry = nullptr;
    IRBlock* left = nullptr;
    IRBlock* right = nullptr;
    IRBlock* merge = nullptr;
    IRParam* condition = nullptr;

    void begin(IRBuilder& builder, IRModule* module, IRType* resultType)
    {
        builder.setInsertInto(module);
        func = builder.createFunc();
        IRType* paramTypes[] = {builder.getBoolType()};
        func->setFullType(builder.getFuncType(1, paramTypes, resultType));
        builder.addNameHintDecoration(func, UnownedStringSlice("f"));
        builder.addKeepAliveDecoration(func);

        builder.setInsertInto(func);
        entry = builder.emitBlock();
        condition = builder.emitParam(builder.getBoolType());
        left = builder.emitBlock();
        right = builder.emitBlock();
        merge = builder.emitBlock();
        builder.setInsertInto(entry);
    }

    void finishEntry(IRBuilder& builder)
    {
        builder.setInsertInto(entry);
        builder.emitIfElse(condition, left, right, merge);
    }

    void finishArm(IRBuilder& builder, IRBlock* block, List<IRInst*> const& args)
    {
        builder.setInsertInto(block);
        builder.emitBranch(merge, args.getCount(), args.getBuffer());
    }
};

/// Return a `TargetProgram` for the default target, so peephole takes its dominance-based path
/// (`useFastAnalysis` is false when a target with full optimization is present).
TargetProgram* getDefaultTargetProgram(StaticUnitTestEnv& env, const char* moduleName)
{
    auto targetModule = env.checkModuleFromSource(moduleName, "");
    SLANG_RELEASE_ASSERT(targetModule);
    auto linkage = targetModule->getLinkage();
    SLANG_RELEASE_ASSERT(linkage->targets.getCount() == 1);
    return targetModule->getTargetProgram(linkage->targets[0]);
}

} // namespace

// Consider `int a; if (c) p = a; else { int b; p = b; } return p == a;`. On the `left` edge `p` is
// `a`, so the comparison is true there. Replacing `p` with a fresh value would make it arbitrary.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenFrozenInputIsObservedElsewhere)
{
    for (bool withTarget : {true, false})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), builder.getBoolType());
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishEntry(builder);
        d.finishArm(builder, d.left, {a});
        builder.setInsertInto(d.right);
        auto b = builder.emitLoadFromUninitializedMemory(intType);
        d.finishArm(builder, d.right, {b});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto eq = builder.emitEql(p, a);
        builder.emitReturn(eq);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        auto target = withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetA") : nullptr;
        peepholeOptimize(target, d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(eq->getOperand(0) == p);
        SLANG_CHECK(countUninitializedLoads(d.entry) == 1);
        SLANG_CHECK(countUninitializedLoads(d.merge) == 0);
    }
}

// Consider `int p; if (c) { int a; p = a; } else { int b; p = b; } return p;`. Nothing but `p`
// observes `a` or `b`, so `p` is undefined on every path and becomes one fresh value. We place it
// in the entry block, which runs once per invocation and dominates every use of `p`.
SLANG_UNIT_TEST(irPeepholeParamReplacesUnobservedFrozenInputsWithEntryValue)
{
    for (bool withTarget : {true, false})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        builder.setInsertInto(d.left);
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishArm(builder, d.left, {a});
        builder.setInsertInto(d.right);
        auto b = builder.emitLoadFromUninitializedMemory(intType);
        d.finishArm(builder, d.right, {b});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto ret = builder.emitReturn(p);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        auto target = withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetB") : nullptr;
        SLANG_CHECK(peepholeOptimize(target, d.func));

        SLANG_CHECK(isValidModule(fixture.getModule()));
        auto replacement = ret->getOperand(0);
        SLANG_CHECK(replacement->getOp() == kIROp_LoadFromUninitializedMemory);
        SLANG_CHECK(replacement->getParent() == d.entry);
        SLANG_CHECK(replacement->getFullType() == p->getFullType());
        SLANG_CHECK(!p->hasUses());
    }
}

// A debug-only use of an input does not observe its value for correlation purposes, so it does not
// block the replacement. We run the rule on the param alone first, so the `DebugValue` is still
// present and must remain valid. A full peephole run then removes it, because peephole already
// treats a `DebugValue` of an undefined value as a no-op.
SLANG_UNIT_TEST(irPeepholeParamIgnoresDebugOnlyUsesOfFrozenInputs)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    auto intType = builder.getIntType();

    builder.setInsertInto(fixture.getModule());
    auto debugSource =
        builder.emitDebugSource(UnownedStringSlice("test.slang"), UnownedStringSlice(), false);
    auto line = builder.getIntValue(builder.getUIntType(), 1);
    auto col = builder.getIntValue(builder.getUIntType(), 1);

    Diamond d;
    d.begin(builder, fixture.getModule(), intType);
    builder.setInsertInto(fixture.getModule());
    auto debugFunction = builder.emitDebugFunction(
        builder.getStringValue(UnownedStringSlice("f")),
        line,
        col,
        debugSource,
        d.func->getDataType());
    builder.setInsertInto(d.entry);
    auto debugVar = builder.emitDebugVar(intType, debugSource, line, col, debugFunction);
    auto a = builder.emitLoadFromUninitializedMemory(intType);
    auto debugValue = builder.emitDebugValue(debugVar, a);
    d.finishEntry(builder);
    d.finishArm(builder, d.left, {a});
    builder.setInsertInto(d.right);
    auto b = builder.emitLoadFromUninitializedMemory(intType);
    d.finishArm(builder, d.right, {b});
    builder.setInsertInto(d.merge);
    auto p = builder.emitParam(intType);
    auto ret = builder.emitReturn(p);
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    SLANG_CHECK(peepholeOptimizeInst(nullptr, fixture.getModule(), p));
    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(ret->getOperand(0)->getOp() == kIROp_LoadFromUninitializedMemory);
    SLANG_CHECK(ret->getOperand(0)->getParent() == d.entry);
    SLANG_CHECK(debugValue->getParent() == d.entry);
    SLANG_CHECK(debugValue->getOperand(1) == a);

    peepholeOptimize(nullptr, d.func);
    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(debugValue->getParent() == nullptr);
}

// Consider `int a; int p = a; for (...) { ... } return p;`, whose loop header param receives `a`
// from the entry and itself from the back edge. Every iteration sees the same `a`, so we forward
// `a` and create no value in the header. Forwarding a block-local value needs a dominance check,
// which the fast analysis used without a target skips, so in that mode the param stays.
SLANG_UNIT_TEST(irPeepholeParamForwardsLoopCarriedFrozenValue)
{
    for (bool withTarget : {true, false})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        builder.setInsertInto(fixture.getModule());
        auto func = builder.createFunc();
        IRType* paramTypes[] = {builder.getBoolType()};
        func->setFullType(builder.getFuncType(1, paramTypes, intType));
        builder.addKeepAliveDecoration(func);
        builder.setInsertInto(func);
        auto entry = builder.emitBlock();
        auto c = builder.emitParam(builder.getBoolType());
        auto header = builder.emitBlock();
        auto body = builder.emitBlock();
        auto exit = builder.emitBlock();

        builder.setInsertInto(entry);
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        IRInst* entryArgs[] = {a};
        builder.emitLoop(header, exit, body, 1, entryArgs);
        builder.setInsertInto(header);
        auto p = builder.emitParam(intType);
        builder.emitIfElse(c, body, exit, body);
        builder.setInsertInto(body);
        IRInst* backArgs[] = {p};
        builder.emitBranch(header, 1, backArgs);
        builder.setInsertInto(exit);
        auto ret = builder.emitReturn(p);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        auto target = withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetD") : nullptr;
        peepholeOptimize(target, func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(countUninitializedLoads(header) == 0);
        SLANG_CHECK(countUninitializedLoads(entry) == 1);
        IRInst* expected = withTarget ? (IRInst*)a : (IRInst*)p;
        SLANG_CHECK(ret->getOperand(0) == expected);
    }
}

// An input that the same branch also passes to another param slot is observed there, so replacing
// either param with a fresh value would break the correlation between the two.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenInputFeedsAnotherSlot)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    auto intType = builder.getIntType();

    Diamond d;
    d.begin(builder, fixture.getModule(), intType);
    d.finishEntry(builder);
    builder.setInsertInto(d.left);
    auto a = builder.emitLoadFromUninitializedMemory(intType);
    d.finishArm(builder, d.left, {a, a});
    builder.setInsertInto(d.right);
    auto b = builder.emitLoadFromUninitializedMemory(intType);
    d.finishArm(builder, d.right, {b, b});
    builder.setInsertInto(d.merge);
    auto p0 = builder.emitParam(intType);
    auto p1 = builder.emitParam(intType);
    auto sum = builder.emitAdd(intType, p0, p1);
    builder.emitReturn(sum);
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetE"), d.func);

    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(sum->getOperand(0) == p0);
    SLANG_CHECK(sum->getOperand(1) == p1);
    SLANG_CHECK(countUninitializedLoads(d.entry) == 0);
}

// `Poison` is deduplicated per type, so all-poison inputs are one instruction and the param is
// forwarded to it. A mix of poison and frozen inputs keeps frozen-value semantics: the result is a
// fresh `LoadFromUninitializedMemory`, unless the frozen input is observed elsewhere.
SLANG_UNIT_TEST(irPeepholeParamHandlesPoisonInputs)
{
    // All poison.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        builder.setInsertInto(d.left);
        auto poison = builder.getPoison(intType);
        d.finishArm(builder, d.left, {poison});
        builder.setInsertInto(d.right);
        SLANG_CHECK_ABORT(builder.getPoison(intType) == poison);
        d.finishArm(builder, d.right, {poison});
        builder.setInsertInto(d.merge);
        builder.emitParam(intType);
        auto ret = builder.emitReturn(d.merge->getFirstParam());
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetF1"), d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(ret->getOperand(0) == poison);
        SLANG_CHECK(countUninitializedLoads(d.entry) == 0);
    }

    // Mixed poison and an unobserved frozen input.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        builder.setInsertInto(d.left);
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishArm(builder, d.left, {a});
        builder.setInsertInto(d.right);
        d.finishArm(builder, d.right, {builder.getPoison(intType)});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto ret = builder.emitReturn(p);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetF2"), d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(ret->getOperand(0)->getOp() == kIROp_LoadFromUninitializedMemory);
        SLANG_CHECK(ret->getOperand(0)->getParent() == d.entry);
    }

    // Mixed poison and a frozen input that is also compared with the param.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), builder.getBoolType());
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishEntry(builder);
        d.finishArm(builder, d.left, {a});
        builder.setInsertInto(d.right);
        d.finishArm(builder, d.right, {builder.getPoison(intType)});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto eq = builder.emitEql(p, a);
        builder.emitReturn(eq);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetF3"), d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(eq->getOperand(0) == p);
    }
}

// The fresh value goes in the entry block, so its type must be available there. Here the param's
// type is `extractExistentialType(e)`, which the builder hoists into the block that defines `e`.
// That block is not the entry block, so the param is kept rather than moving its type.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenTypeIsNotAvailableAtEntry)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());

    builder.setInsertInto(fixture.getModule());
    auto interfaceType = builder.createInterfaceType(0, nullptr);
    auto func = builder.createFunc();
    IRType* paramTypes[] = {builder.getBoolType()};
    func->setFullType(builder.getFuncType(1, paramTypes, builder.getVoidType()));
    builder.addKeepAliveDecoration(func);
    builder.setInsertInto(func);
    auto entry = builder.emitBlock();
    auto c = builder.emitParam(builder.getBoolType());
    auto typeBlock = builder.emitBlock();
    auto left = builder.emitBlock();
    auto right = builder.emitBlock();
    auto merge = builder.emitBlock();

    builder.setInsertInto(entry);
    builder.emitBranch(typeBlock);
    builder.setInsertInto(typeBlock);
    auto e = builder.emitLoadFromUninitializedMemory(interfaceType);
    auto localType = builder.emitExtractExistentialType(e);
    SLANG_CHECK_ABORT(localType->getParent() == typeBlock);
    builder.emitIfElse(c, left, right, merge);
    builder.setInsertInto(left);
    IRInst* leftArgs[] = {builder.emitLoadFromUninitializedMemory(localType)};
    builder.emitBranch(merge, 1, leftArgs);
    builder.setInsertInto(right);
    IRInst* rightArgs[] = {builder.emitLoadFromUninitializedMemory(localType)};
    builder.emitBranch(merge, 1, rightArgs);
    builder.setInsertInto(merge);
    auto p = builder.emitParam(localType);
    auto var = builder.emitVar(localType);
    auto store = builder.emitStore(var, p);
    builder.emitReturn();
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetG"), func);

    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(store->getOperand(1) == p);
    SLANG_CHECK(countUninitializedLoads(entry) == 0);
}

// The front end's uninitialized-use check runs after the prelinking peephole passes and reports
// each undefined value on its own. Prelinking runs therefore keep params whose inputs are all
// undefined, while still forwarding the one defined value of a mixed param.
SLANG_UNIT_TEST(irPeepholeParamKeepsUndefinedParamsWhenPrelinking)
{
    // Distinct frozen inputs.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        builder.setInsertInto(d.left);
        d.finishArm(builder, d.left, {builder.emitLoadFromUninitializedMemory(intType)});
        builder.setInsertInto(d.right);
        d.finishArm(builder, d.right, {builder.emitLoadFromUninitializedMemory(intType)});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto ret = builder.emitReturn(p);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(nullptr, d.func, PeepholeOptimizationOptions::getPrelinking());

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(ret->getOperand(0) == p);
    }

    // All poison.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        d.finishArm(builder, d.left, {builder.getPoison(intType)});
        d.finishArm(builder, d.right, {builder.getPoison(intType)});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto ret = builder.emitReturn(p);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(nullptr, d.func, PeepholeOptimizationOptions::getPrelinking());

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(ret->getOperand(0) == p);
    }

    // One frozen input and one constant.
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        d.finishEntry(builder);
        builder.setInsertInto(d.left);
        d.finishArm(builder, d.left, {builder.emitLoadFromUninitializedMemory(intType)});
        auto seven = builder.getIntValue(intType, 7);
        d.finishArm(builder, d.right, {seven});
        builder.setInsertInto(d.merge);
        builder.emitParam(intType);
        auto ret = builder.emitReturn(d.merge->getFirstParam());
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(nullptr, d.func, PeepholeOptimizationOptions::getPrelinking());

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(ret->getOperand(0) == seven);
    }
}

// A param whose only incoming value is itself has no value to forward and no undefined input. The
// rule leaves it alone and reports no change, so peephole's fixpoint loop terminates.
SLANG_UNIT_TEST(irPeepholeParamLeavesSelfOnlyParamAlone)
{
    for (bool withTarget : {true, false})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        builder.setInsertInto(fixture.getModule());
        auto func = builder.createFunc();
        func->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addKeepAliveDecoration(func);
        builder.setInsertInto(func);
        auto entry = builder.emitBlock();
        auto spin = builder.emitBlock();
        builder.setInsertInto(entry);
        builder.emitReturn();
        builder.setInsertInto(spin);
        auto p = builder.emitParam(intType);
        IRInst* args[] = {p};
        auto branch = builder.emitBranch(spin, 1, args);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        auto target = withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetI") : nullptr;
        SLANG_CHECK(!peepholeOptimize(target, func));

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(as<IRUnconditionalBranch>(branch)->getArg(0) == p);
    }
}

// A param with one defined value and undefined inputs is forwarded to the defined value. A frozen
// input that is also observed elsewhere is refined to that value first, so `p == a` becomes
// `7 == 7` rather than comparing the forwarded value with an independent arbitrary one.
SLANG_UNIT_TEST(irPeepholeParamForwardsSingleDefinedValueAndRefinesObservedInput)
{
    for (bool observed : {false, true})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), builder.getBoolType());
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishEntry(builder);
        d.finishArm(builder, d.left, {a});
        auto seven = builder.getIntValue(intType, 7);
        d.finishArm(builder, d.right, {seven});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        auto eq = builder.emitEql(p, observed ? a : seven);
        builder.emitReturn(eq);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetX"), d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(eq->getOperand(0) == seven);
        SLANG_CHECK(eq->getOperand(1) == seven);
    }
}

// A frozen input is refined only to a value that cannot be poison. Here `x` is computed in the entry
// block after `a` and dominates every use of `a`, but an arbitrary instruction result may be poison
// (or may depend on a poison argument), so the param is kept rather than refining `a` to `x`.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenRefinementValueIsBlockLocal)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    auto intType = builder.getIntType();

    Diamond d;
    d.begin(builder, fixture.getModule(), builder.getBoolType());
    auto a = builder.emitLoadFromUninitializedMemory(intType);
    auto x = builder.emitAdd(
        intType,
        builder.emitCast(intType, d.condition),
        builder.getIntValue(intType, 2));
    d.finishEntry(builder);
    d.finishArm(builder, d.left, {x});
    d.finishArm(builder, d.right, {a});
    builder.setInsertInto(d.merge);
    auto p = builder.emitParam(intType);
    auto eq = builder.emitEql(p, a);
    builder.emitReturn(eq);
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetY"), d.func);

    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(eq->getOperand(0) == p);
    SLANG_CHECK(eq->getOperand(1) == a);
}

// Consider:
//
//     entry(c): d = select(c, 1, poison); a = LoadFromUninitializedMemory<int>(); if (c) ...
//     left:     br merge(d)
//     right:    br merge(a)
//     merge(p): return select(c, p, a)
//
// `d` dominates `a` and every use of it, but `d` is poison when `c` is false. On that path the
// function returns `a`, an arbitrary but defined value. Refining every use of `a` to `d` would
// return poison there, so the param is kept.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenRefinementValueMayBePoison)
{
    for (bool withTarget : {true, false})
    {
        StaticUnitTestEnv env(unitTestContext);
        IRFixtureBuilder fixture(env.getSessionImpl());
        IRBuilder builder(fixture.getModule());
        auto intType = builder.getIntType();

        Diamond d;
        d.begin(builder, fixture.getModule(), intType);
        IRInst* selectArgs[] = {
            d.condition,
            builder.getIntValue(intType, 1),
            builder.getPoison(intType)};
        auto dValue = builder.emitIntrinsicInst(intType, kIROp_Select, 3, selectArgs);
        auto a = builder.emitLoadFromUninitializedMemory(intType);
        d.finishEntry(builder);
        d.finishArm(builder, d.left, {dValue});
        d.finishArm(builder, d.right, {a});
        builder.setInsertInto(d.merge);
        auto p = builder.emitParam(intType);
        IRInst* resultArgs[] = {d.condition, p, a};
        auto result = builder.emitIntrinsicInst(intType, kIROp_Select, 3, resultArgs);
        builder.emitReturn(result);
        SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

        auto target = withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetP") : nullptr;
        peepholeOptimize(target, d.func);

        SLANG_CHECK(isValidModule(fixture.getModule()));
        SLANG_CHECK(result->getOperand(1) == p);
        SLANG_CHECK(result->getOperand(2) == a);
    }
}

// A global shader parameter is bound by the application, has no operands that could carry poison,
// and is fixed for the whole invocation, so an observed frozen input is refined to it. A
// specialization constant with a default value is excluded, because the default is an arbitrary
// lowered expression; that param is kept.
SLANG_UNIT_TEST(irPeepholeParamRefinesObservedInputToGlobalParam)
{
    for (bool hasDefaultValue : {false, true})
    {
        for (bool withTarget : {true, false})
        {
            StaticUnitTestEnv env(unitTestContext);
            IRFixtureBuilder fixture(env.getSessionImpl());
            IRBuilder builder(fixture.getModule());
            auto intType = builder.getIntType();

            builder.setInsertInto(fixture.getModule());
            auto global = builder.createGlobalParam(intType);
            if (hasDefaultValue)
                builder.addDefaultValueDecoration(global, builder.getIntValue(intType, 3));

            Diamond d;
            d.begin(builder, fixture.getModule(), builder.getBoolType());
            auto a = builder.emitLoadFromUninitializedMemory(intType);
            d.finishEntry(builder);
            d.finishArm(builder, d.left, {a});
            d.finishArm(builder, d.right, {global});
            builder.setInsertInto(d.merge);
            auto p = builder.emitParam(intType);
            auto eq = builder.emitEql(p, a);
            builder.emitReturn(eq);
            SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

            auto target =
                withTarget ? getDefaultTargetProgram(env, "peepholeParamTargetG") : nullptr;
            peepholeOptimize(target, d.func);

            SLANG_CHECK(isValidModule(fixture.getModule()));
            IRInst* expectedLeft = hasDefaultValue ? (IRInst*)p : global;
            IRInst* expectedRight = hasDefaultValue ? (IRInst*)a : global;
            SLANG_CHECK(eq->getOperand(0) == expectedLeft);
            SLANG_CHECK(eq->getOperand(1) == expectedRight);
        }
    }
}

// The refinement is refused when the defined value can change while the frozen input stays fixed.
// Here `a` is defined before a loop and `x` is computed in the loop header, which dominates the
// merge param `p = phi(x, a)`. Forwarding `p` to `x` would need `a` to become `x`, but `x` changes
// on each iteration while `a` must not, and `x` is not a value that is guaranteed to be non-poison
// either, so the param is kept.
SLANG_UNIT_TEST(irPeepholeParamKeepsParamWhenRefinementValueVariesInLoop)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    auto intType = builder.getIntType();

    builder.setInsertInto(fixture.getModule());
    auto func = builder.createFunc();
    IRType* paramTypes[] = {builder.getBoolType()};
    func->setFullType(builder.getFuncType(1, paramTypes, builder.getBoolType()));
    builder.addKeepAliveDecoration(func);
    builder.setInsertInto(func);
    auto entry = builder.emitBlock();
    auto c = builder.emitParam(builder.getBoolType());
    auto header = builder.emitBlock();
    auto left = builder.emitBlock();
    auto right = builder.emitBlock();
    auto merge = builder.emitBlock();
    auto exit = builder.emitBlock();

    builder.setInsertInto(entry);
    auto a = builder.emitLoadFromUninitializedMemory(intType);
    IRInst* entryArgs[] = {builder.getIntValue(intType, 0)};
    builder.emitLoop(header, exit, header, 1, entryArgs);
    builder.setInsertInto(header);
    auto i = builder.emitParam(intType);
    auto x = builder.emitAdd(intType, i, builder.getIntValue(intType, 1));
    builder.emitIfElse(c, left, right, merge);
    builder.setInsertInto(left);
    IRInst* leftArgs[] = {x};
    builder.emitBranch(merge, 1, leftArgs);
    builder.setInsertInto(right);
    IRInst* rightArgs[] = {a};
    builder.emitBranch(merge, 1, rightArgs);
    builder.setInsertInto(merge);
    auto p = builder.emitParam(intType);
    auto eq = builder.emitEql(p, a);
    auto continueBlock = builder.emitBlock();
    builder.setInsertInto(merge);
    builder.emitIfElse(eq, exit, continueBlock, exit);
    builder.setInsertInto(continueBlock);
    IRInst* backArgs[] = {x};
    builder.emitBranch(header, 1, backArgs);
    builder.setInsertInto(exit);
    builder.emitReturn(builder.getBoolValue(true));
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    peepholeOptimize(getDefaultTargetProgram(env, "peepholeParamTargetZ"), func);

    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(eq->getOperand(0) == p);
    SLANG_CHECK(eq->getOperand(1) == a);
}

// Prelinking runs forward the single defined value without refining the frozen input, so the
// front end's uninitialized-use check still sees `a`.
SLANG_UNIT_TEST(irPeepholeParamForwardsWithoutRefiningWhenPrelinking)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    auto intType = builder.getIntType();

    Diamond d;
    d.begin(builder, fixture.getModule(), builder.getBoolType());
    auto a = builder.emitLoadFromUninitializedMemory(intType);
    d.finishEntry(builder);
    d.finishArm(builder, d.left, {a});
    auto seven = builder.getIntValue(intType, 7);
    d.finishArm(builder, d.right, {seven});
    builder.setInsertInto(d.merge);
    builder.emitParam(intType);
    auto eq = builder.emitEql(d.merge->getFirstParam(), a);
    builder.emitReturn(eq);
    SLANG_CHECK_ABORT(isValidModule(fixture.getModule()));

    peepholeOptimize(nullptr, d.func, PeepholeOptimizationOptions::getPrelinking());

    SLANG_CHECK(isValidModule(fixture.getModule()));
    SLANG_CHECK(eq->getOperand(0) == seven);
    SLANG_CHECK(eq->getOperand(1) == a);
}
