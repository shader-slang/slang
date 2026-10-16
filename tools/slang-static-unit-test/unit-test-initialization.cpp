#include "slang/slang-ir-use-uninitialized-values.h"
#include "slang/slang-ir-util.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

// Source tests cover lowering and diagnostics. These tests call the analysis
// directly to force every small work cutoff and compare surviving IR load
// identities. They check that incomplete refinement cannot suppress warnings, that
// the shared baseline matches a per-variable reference, and that repacking preserves identities.

using namespace Slang;

namespace
{
// Build two if statements testing the same condition: the first stores, the second loads.
// The first variableCount - conditionallyStoredVariableCount variables are stored on
// both arms of the first if; the remaining variables are stored only on its true arm.
// The counts satisfy 1 <= conditionallyStoredVariableCount <= variableCount.
// The last variable also has an unconditional load that must keep warning.
// Tests compare surviving IR instructions to detect incorrect bit-to-variable mapping.
struct VariableInitializationFixture
{
    RefPtr<IRModule> module;
    IRFunc* func;
    List<VariableInitializationInfo> variables;
    List<IRInst*> guardedLoads;
    IRInst* unconditionalLoad;
    IRInst* independentCondition;
    IRIfElse* loadBranch;

    VariableInitializationFixture(
        Session* session,
        Index variableCount,
        Index conditionallyStoredVariableCount)
        : module(IRModule::create(session))
    {
        IRBuilder builder(module);
        builder.setInsertInto(module);
        func = builder.createFunc();
        IRType* boolType = builder.getBoolType();
        IRType* paramTypes[] = {boolType, boolType};
        func->setFullType(builder.getFuncType(2, paramTypes, builder.getVoidType()));
        builder.setInsertInto(func);
        auto entry = builder.emitBlock();
        auto trueStoreBlock = builder.emitBlock();
        auto falseStoreBlock = builder.emitBlock();
        auto mergeBlock = builder.emitBlock();
        auto loadBlock = builder.emitBlock();
        auto exitBlock = builder.emitBlock();
        builder.setInsertInto(entry);
        auto condition = builder.emitParam(boolType);
        independentCondition = builder.emitParam(boolType);
        List<IRInst*> irVariables;
        for (Index i = 0; i < variableCount; i++)
            irVariables.add(builder.emitVar(builder.getIntType()));
        builder.emitIfElse(condition, trueStoreBlock, falseStoreBlock, mergeBlock);
        builder.setInsertInto(trueStoreBlock);
        auto value = builder.getIntValue(builder.getIntType(), 1);
        for (auto variable : irVariables)
            builder.emitStore(variable, value);
        builder.emitBranch(mergeBlock);
        builder.setInsertInto(falseStoreBlock);
        for (Index i = 0; i < variableCount - conditionallyStoredVariableCount; i++)
            builder.emitStore(irVariables[i], value);
        builder.emitBranch(mergeBlock);
        builder.setInsertInto(mergeBlock);
        loadBranch = builder.emitIfElse(condition, loadBlock, exitBlock, exitBlock);
        builder.setInsertInto(loadBlock);
        for (Index i = 0; i < variableCount; i++)
        {
            VariableInitializationInfo variable;
            variable.blocksWithStore.add(trueStoreBlock);
            if (i < variableCount - conditionallyStoredVariableCount)
                variable.blocksWithStore.add(falseStoreBlock);
            auto load = builder.emitLoad(irVariables[i]);
            guardedLoads.add(load);
            variable.loads.add(load);
            variables.add(_Move(variable));
        }
        builder.emitBranch(exitBlock);
        builder.setInsertInto(exitBlock);
        unconditionalLoad = builder.emitLoad(irVariables.getLast());
        variables.getLast().loads.add(unconditionalLoad);
        builder.emitReturn();
    }
};

// Compute store-free reachability separately for each variable. This reference
// uses only CFG edges and sets, without shared masks or refinement state.
static List<IRInst*> getReferenceUninitializedLoads(
    IRFunc* func,
    const VariableInitializationInfo& variable)
{
    using Edge = KeyValuePair<IRBlock*, IRBlock*>;
    List<Edge> worklist;
    HashSet<Edge> visited;
    HashSet<IRBlock*> reachable;
    worklist.add(Edge(nullptr, func->getFirstBlock()));
    while (worklist.getCount())
    {
        auto edge = worklist.getLast();
        worklist.removeLast();
        auto predecessor = edge.key;
        auto block = edge.value;
        if (!visited.add(edge) || variable.suppressedBreakBlocks.contains(block))
            continue;
        reachable.add(block);
        if (variable.blocksWithStore.contains(block))
            continue;

        // A Boolean phi supplied by this edge can exclude one outgoing branch.
        IRBlock* excluded = nullptr;
        if (auto branch = as<IRIfElse>(block->getTerminator()))
        {
            auto param = as<IRParam>(branch->getCondition());
            if (predecessor && param && param->getParent() == block)
            {
                auto incoming = cast<IRUnconditionalBranch>(predecessor->getTerminator());
                auto value = as<IRBoolLit>(incoming->getArg(getParamIndexInBlock(param)));
                if (value)
                    excluded = value->getValue() ? branch->getFalseBlock() : branch->getTrueBlock();
            }
        }
        for (auto successor : block->getSuccessors())
            if (successor != excluded)
                worklist.add(Edge(block, successor));
    }
    List<IRInst*> loads;
    for (auto load : variable.loads)
        if (reachable.contains(cast<IRBlock>(load->getParent())))
            loads.add(load);
    return loads;
}

// Build a diamond feeding a Boolean phi and a loop with a second incoming false
// edge. The right arm has a detour, and physical block order is not traversal order.
// Loads at block entry precede stores; suppression applies before block entry.
struct InitializationGraphFixture
{
    RefPtr<IRModule> module;
    IRFunc* func;
    List<VariableInitializationInfo> variables;

    InitializationGraphFixture(Session* session, Index variableCount)
        : module(IRModule::create(session))
    {
        IRBuilder builder(module);
        builder.setInsertInto(module);
        func = builder.createFunc();
        IRType* paramTypes[] = {builder.getBoolType(), builder.getBoolType()};
        func->setFullType(builder.getFuncType(2, paramTypes, builder.getVoidType()));
        builder.setInsertInto(func);
        auto entry = builder.emitBlock();
        auto exit = builder.emitBlock();
        auto guarded = builder.emitBlock();
        auto back = builder.emitBlock();
        auto left = builder.emitBlock();
        auto right = builder.emitBlock();
        auto header = builder.emitBlock();
        auto skip = builder.emitBlock();
        auto latch = builder.emitBlock();
        auto detour = builder.emitBlock();
        builder.setInsertInto(entry);
        auto chooseLeft = builder.emitParam(builder.getBoolType());
        auto again = builder.emitParam(builder.getBoolType());
        List<IRInst*> irVariables;
        for (Index i = 0; i < variableCount; i++)
        {
            irVariables.add(builder.emitVar(builder.getIntType()));
            VariableInitializationInfo variable;
            if (i % 3 != 2)
                variable.blocksWithStore.add(left);
            if (i % 3 == 1)
                variable.blocksWithStore.add(right);
            if (i % 3 == 2)
                variable.blocksWithStore.add(back);
            if (i % 5 == 0)
                variable.suppressedBreakBlocks.add(exit);
            if (i % 7 == 0)
                variable.suppressedBreakBlocks.add(guarded);
            variables.add(_Move(variable));
        }
        builder.emitIfElse(chooseLeft, left, right, header);
        IRInst* trueValue = builder.getBoolValue(true);
        IRInst* falseValue = builder.getBoolValue(false);
        auto one = builder.getIntValue(builder.getIntType(), 1);
        for (auto block : {left, right, back})
        {
            builder.setInsertInto(block);
            for (Index i = 0; i < variableCount; i++)
            {
                if (block == back)
                    variables[i].loads.add(builder.emitLoad(irVariables[i]));
                if (variables[i].blocksWithStore.contains(block))
                    builder.emitStore(irVariables[i], one);
            }
            if (block == right)
                builder.emitBranch(detour);
            else
                builder.emitBranch(header, 1, block == left ? &trueValue : &falseValue);
        }
        builder.setInsertInto(detour);
        builder.emitBranch(header, 1, &falseValue);
        builder.setInsertInto(header);
        auto phi = builder.emitParam(builder.getBoolType());
        for (auto block : {header, guarded, exit})
        {
            builder.setInsertInto(block);
            for (Index i = 0; i < variableCount; i++)
                variables[i].loads.add(builder.emitLoad(irVariables[i]));
            if (block == header)
                builder.emitIfElse(phi, guarded, skip, latch);
            else if (block == guarded)
                builder.emitBranch(latch);
            else
                builder.emitReturn();
        }
        builder.setInsertInto(skip);
        builder.emitBranch(latch);
        builder.setInsertInto(latch);
        builder.emitIfElse(again, back, exit, exit);
    }
};
} // namespace

// Compare identities across word boundaries, including paths pruned by phi arguments,
// loop revisits, block-entry loads before stores, and variable-specific suppression.
SLANG_UNIT_TEST(initializationSharedBaselineMatchesReference)
{
    StaticUnitTestEnv env(unitTestContext);
    for (Index variableCount : {65, 129})
    {
        InitializationGraphFixture fixture(env.getSessionImpl(), variableCount);
        auto variables = fixture.variables;
        removeInitializedLoads(fixture.func, variables, 0);
        SLANG_CHECK_ABORT(variables.getCount() == variableCount);
        for (Index i = 0; i < variableCount; i++)
        {
            auto expected = getReferenceUninitializedLoads(fixture.func, fixture.variables[i]);
            SLANG_CHECK(variables[i].loads.getCount() == expected.getCount());
            for (auto load : fixture.variables[i].loads)
                SLANG_CHECK(variables[i].loads.contains(load) == expected.contains(load));
        }
        // Variable 3 is stored only on the true incoming edge. Its header load is
        // unsafe, but the phi rules exclude the unstored edge from its guarded load.
        SLANG_CHECK(variables[3].loads.contains(fixture.variables[3].loads[1]));
        SLANG_CHECK(!variables[3].loads.contains(fixture.variables[3].loads[2]));
        // Variable 2 is read in the back-edge block before that block stores it.
        SLANG_CHECK(variables[2].loads.contains(fixture.variables[2].loads[0]));
        SLANG_CHECK(variables[2].loads.contains(fixture.variables[2].loads[2]));
        SLANG_CHECK(!variables[5].loads.contains(fixture.variables[5].loads[3]));
        SLANG_CHECK(!variables[14].loads.contains(fixture.variables[14].loads[2]));
    }
}

// Check that remapping unresolved variables preserves their list indices and the
// last variable's unconditional load, including when the mask word count shrinks.
SLANG_UNIT_TEST(initializationBatchPreservesVariableIndices)
{
    StaticUnitTestEnv env(unitTestContext);
    for (Index conditionallyStoredVariableCount : {1, 65, 128})
    {
        VariableInitializationFixture fixture(
            env.getSessionImpl(),
            128,
            conditionallyStoredVariableCount);
        auto variables = fixture.variables;
        removeInitializedLoads(fixture.func, variables);
        SLANG_CHECK(variables.getCount() == 128);
        for (Index i = 0; i < 127; i++)
            SLANG_CHECK(variables[i].loads.getCount() == 0);
        SLANG_CHECK_ABORT(variables[127].loads.getCount() == 1);
        SLANG_CHECK(variables[127].loads[0] == fixture.unconditionalLoad);
    }
}

// Check that condition tracking removes all remaining guarded loads together or leaves them intact.
SLANG_UNIT_TEST(initializationConditionWalkRemovesLoadsTogether)
{
    StaticUnitTestEnv env(unitTestContext);
    VariableInitializationFixture fixture(env.getSessionImpl(), 128, 65);
    bool sawIncompleteRefinement = false;
    bool sawCompleteRefinement = false;
    // Check every cutoff, including partial traversal and a completed traversal
    // whose result scan exceeds the remaining work. A condition walk removes the
    // remaining guarded loads only after traversal finishes and the full scan fits the limit.
    for (Index maxWork = 0; maxWork <= 1024; maxWork++)
    {
        auto variables = fixture.variables;
        removeInitializedLoads(fixture.func, variables, maxWork);
        for (Index i = 0; i < 63; i++)
            SLANG_CHECK(variables[i].loads.getCount() == 0);
        SLANG_CHECK(variables[127].loads.contains(fixture.unconditionalLoad));
        bool refinementCompleted = variables[63].loads.getCount() == 0;
        sawCompleteRefinement |= refinementCompleted;
        sawIncompleteRefinement |= !refinementCompleted;
        for (Index i = 63; i < 128; i++)
        {
            SLANG_CHECK(
                variables[i].loads.contains(fixture.guardedLoads[i]) == !refinementCompleted);
            SLANG_CHECK(
                variables[i].loads.getCount() ==
                (refinementCompleted ? 0 : 1) + (i == 127 ? 1 : 0));
        }
    }
    SLANG_CHECK(sawIncompleteRefinement);
    SLANG_CHECK(sawCompleteRefinement);
}

// Check that disabling refinement still completes initialization analysis for every variable.
SLANG_UNIT_TEST(initializationBaselineIgnoresRefinementLimit)
{
    StaticUnitTestEnv env(unitTestContext);
    VariableInitializationFixture fixture(env.getSessionImpl(), 129, 65);
    // Prevent condition refinement by making the second branch independent.
    fixture.loadBranch->setOperand(0, fixture.independentCondition);
    for (Index maxWork : {0, 1, 1024})
    {
        auto variables = fixture.variables;
        removeInitializedLoads(fixture.func, variables, maxWork);
        for (Index i = 0; i < 129; i++)
        {
            SLANG_CHECK(variables[i].loads.getCount() == (i < 64 ? 0 : i == 128 ? 2 : 1));
            if (i >= 64)
                SLANG_CHECK(variables[i].loads.contains(fixture.guardedLoads[i]));
        }
        SLANG_CHECK(variables[128].loads.contains(fixture.unconditionalLoad));
    }
}

// Check that suppression masks do not hide neighboring variables' loads, even after remapping.
SLANG_UNIT_TEST(initializationSuppressionMasksStayVariableSpecific)
{
    StaticUnitTestEnv env(unitTestContext);
    VariableInitializationFixture fixture(env.getSessionImpl(), 129, 129);
    auto condition = fixture.loadBranch->getCondition();
    auto loadBlock = fixture.loadBranch->getTrueBlock();
    // Supply loop-break suppression directly so this test checks mask propagation.
    // Suppressing variables 1 and 64 must not suppress their neighbors in either word.
    fixture.variables[1].suppressedBreakBlocks.add(loadBlock);
    fixture.variables[64].suppressedBreakBlocks.add(loadBlock);
    fixture.loadBranch->setOperand(0, fixture.independentCondition);
    for (Index maxWork = 0; maxWork <= 1024; maxWork++)
    {
        auto variables = fixture.variables;
        removeInitializedLoads(fixture.func, variables, maxWork);
        for (Index i = 0; i < 129; i++)
            SLANG_CHECK(
                variables[i].loads.contains(fixture.guardedLoads[i]) == (i != 1 && i != 64));
        SLANG_CHECK(variables[128].loads.contains(fixture.unconditionalLoad));
    }

    // Baseline analysis clears the candidate loads for variables 1 and 64, so
    // condition tracking omits their bits. The remaining bits occupy fewer words, while variable
    // indices stay fixed. The unconditional load must still belong to variable 128.
    fixture.loadBranch->setOperand(0, condition);
    auto variables = fixture.variables;
    removeInitializedLoads(fixture.func, variables);
    for (Index i = 0; i < 128; i++)
        SLANG_CHECK(variables[i].loads.getCount() == 0);
    SLANG_CHECK_ABORT(variables[128].loads.getCount() == 1);
    SLANG_CHECK(variables[128].loads[0] == fixture.unconditionalLoad);
}

// One variable has stores and loads guarded separately by a and b. Finishing the
// proof for a must survive exhaustion during b's walk, while the unconditional
// load remains a genuine violation at every cutoff.
SLANG_UNIT_TEST(initializationRetainsEarlierConditionProof)
{
    StaticUnitTestEnv env(unitTestContext);
    auto module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module);
    auto func = builder.createFunc();
    IRType* params[] = {builder.getBoolType(), builder.getBoolType()};
    func->setFullType(builder.getFuncType(2, params, builder.getVoidType()));
    builder.setInsertInto(func);
    auto entry = builder.emitBlock();
    auto storeA = builder.emitBlock();
    auto afterStoreA = builder.emitBlock();
    auto storeB = builder.emitBlock();
    auto afterStoreB = builder.emitBlock();
    auto loadA = builder.emitBlock();
    auto afterLoadA = builder.emitBlock();
    auto loadB = builder.emitBlock();
    auto exit = builder.emitBlock();
    List<IRBlock*> extraLoadBlocks;
    for (Index i = 0; i < 16; i++)
        extraLoadBlocks.add(builder.emitBlock());
    builder.setInsertInto(entry);
    auto a = builder.emitParam(builder.getBoolType());
    auto b = builder.emitParam(builder.getBoolType());
    auto variable = builder.emitVar(builder.getIntType());
    auto one = builder.getIntValue(builder.getIntType(), 1);
    builder.emitIfElse(a, storeA, afterStoreA, afterStoreA);
    builder.setInsertInto(storeA);
    builder.emitStore(variable, one);
    builder.emitBranch(afterStoreA);
    builder.setInsertInto(afterStoreA);
    builder.emitIfElse(b, storeB, afterStoreB, afterStoreB);
    builder.setInsertInto(storeB);
    builder.emitStore(variable, one);
    builder.emitBranch(afterStoreB);
    builder.setInsertInto(afterStoreB);
    builder.emitIfElse(a, loadA, afterLoadA, afterLoadA);
    builder.setInsertInto(loadA);
    auto guardedA = builder.emitLoad(variable);
    // A's proof skips this arm entirely; B's proof must traverse these blocks.
    // This gives a cutoff where A completes but B's separate allowance is too small.
    for (auto next : extraLoadBlocks)
    {
        builder.emitBranch(next);
        builder.setInsertInto(next);
    }
    builder.emitBranch(afterLoadA);
    builder.setInsertInto(afterLoadA);
    builder.emitIfElse(b, loadB, exit, exit);
    builder.setInsertInto(loadB);
    auto guardedB = builder.emitLoad(variable);
    builder.emitBranch(exit);
    builder.setInsertInto(exit);
    auto unguarded = builder.emitLoad(variable);
    builder.emitReturn();

    VariableInitializationInfo original;
    original.blocksWithStore.add(storeA);
    original.blocksWithStore.add(storeB);
    original.loads.add(guardedA);
    original.loads.add(guardedB);
    original.loads.add(unguarded);
    bool sawFirstProofOnly = false;
    bool sawBothProofs = false;
    for (Index maxWork = 0; maxWork <= 512; maxWork++)
    {
        List<VariableInitializationInfo> variables;
        variables.add(original);
        removeInitializedLoads(func, variables, maxWork);
        const auto& loads = variables[0].loads;
        SLANG_CHECK(loads.contains(unguarded));
        bool provedA = !loads.contains(guardedA);
        bool provedB = !loads.contains(guardedB);
        SLANG_CHECK(!provedB || provedA);
        sawFirstProofOnly |= provedA && !provedB;
        sawBothProofs |= provedA && provedB;
    }
    SLANG_CHECK(sawFirstProofOnly);
    SLANG_CHECK(sawBothProofs);
}
