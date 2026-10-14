#include "slang/slang-ir-use-uninitialized-values.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

// Source tests cover lowering and diagnostics. These tests call the analysis
// directly to force every small work cutoff and compare surviving IR load
// identities. They check that incomplete walks cannot suppress warnings, that
// fallback preserves baseline results, and that repacking keeps loads with their variables.

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
} // namespace

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

// Check that an incomplete shared walk falls back to the same per-variable results.
SLANG_UNIT_TEST(initializationFallbackMatchesBaseline)
{
    StaticUnitTestEnv env(unitTestContext);
    VariableInitializationFixture fixture(env.getSessionImpl(), 129, 65);
    // Prevent condition refinement by making the second branch independent.
    fixture.loadBranch->setOperand(0, fixture.independentCondition);
    for (Index maxWork = 0; maxWork <= 1024; maxWork++)
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
