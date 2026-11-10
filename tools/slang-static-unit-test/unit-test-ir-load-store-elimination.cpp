// Tests for load/store redundancy elimination on hand-built IR.
//
// We call `eliminateRedundantLoadStore` directly so earlier optimizations cannot remove the
// address uses and control-flow shapes being tested. The fixtures use pointer parameters rather
// than local variables: an otherwise unused local can lose all its stores through the separate
// dead-variable check, without exercising forwarding or overwrite detection.

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/slang-ir-redundancy-removal.h"
#include "slang/slang-ir-validate.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Create a function and leave `builder` inserting after its entry parameters.
///
/// The parameter and return types must belong to the builder's module. The caller supplies the
/// body and a terminator for every block.
static IRFunc* beginLoadStoreTestFunction(
    IRBuilder& builder,
    IRType* returnType,
    UInt parameterCount,
    IRType* const* parameterTypes)
{
    builder.setInsertInto(builder.getModule());
    auto func = builder.createFunc();
    func->setFullType(builder.getFuncType(parameterCount, parameterTypes, returnType));

    // We emit the signature and parameter definitions together so fixtures cannot accidentally
    // declare one set of parameters and use another.
    builder.setInsertInto(func);
    builder.emitBlock();
    for (UInt i = 0; i < parameterCount; ++i)
        builder.emitParam(parameterTypes[i]);
    return func;
}

/// Count instructions with `op` across every block of `func`.
static UInt countLoadStoreTestInsts(IRFunc* func, IROp op)
{
    UInt count = 0;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == op)
                ++count;
        }
    }
    return count;
}

/// Validate a fixture's IR and stop its test if the fixture violates an IR invariant.
static void checkLoadStoreTestIR(IRModule* module)
{
    DiagnosticSink sink;
    validateIRModule(module, &sink);
    SLANG_CHECK_ABORT(sink.getErrorCount() == 0);
}

// A surviving store to the exact pointer supplies the value of a later load. We inspect the
// return operand as well as the load count so deleting the load without forwarding its value
// cannot satisfy the test.
SLANG_UNIT_TEST(irLoadStoreForwardsMatchingStore)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto func = beginLoadStoreTestFunction(builder, intType, 1, parameterTypes);
    auto address = func->getFirstParam();
    auto storedValue = builder.getIntValue(intType, 7);
    builder.emitStore(address, storedValue);
    auto load = builder.emitLoad(address);
    auto returnInst = builder.emitReturn(load);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 0);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 1);
    SLANG_CHECK(returnInst->getOperand(0) == storedValue);
}

// A store and load through the same array element address can forward even when that address
// is a projection of a struct field. The search compares complete pointer operands, so recording
// only the store's root address would miss the matching load through the projected pointer.
SLANG_UNIT_TEST(irLoadStoreForwardsMatchingProjectedAddress)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());
    auto intType = builder.getIntType();
    auto arrayType = builder.getArrayType(intType, builder.getIntValue(intType, 2));
    auto aggregateType = builder.createStructType();
    auto arrayField = builder.createStructKey();
    builder.createStructField(aggregateType, arrayField, arrayType);

    // We model `output.values[0] = 7; return output.values[0];`, with `output` passed by
    // reference. The ordinary pointer parameter avoids the separate dead-local-store removal.
    IRType* parameterTypes[] = {builder.getPtrType(aggregateType)};
    auto func = beginLoadStoreTestFunction(builder, intType, 1, parameterTypes);
    auto address = func->getFirstParam();
    auto fieldAddress = builder.emitFieldAddress(address, arrayField);
    auto elementAddress = builder.emitElementAddress(fieldAddress, IRIntegerValue(0));
    auto storedValue = builder.getIntValue(intType, 7);
    builder.emitStore(elementAddress, storedValue);
    auto load = builder.emitLoad(elementAddress);
    auto returnInst = builder.emitReturn(load);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_FieldAddress) == 1);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_GetElementPtr) == 1);
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 0);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 1);
    SLANG_CHECK(returnInst->getOperand(0) == storedValue);
}

// A field address differs from the aggregate pointer but overlaps its storage. The field store
// changes part of the aggregate, so the earlier whole-value store cannot supply the later load.
SLANG_UNIT_TEST(irLoadStoreRetainsWholeLoadAfterFieldStore)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());
    auto intType = builder.getIntType();
    auto aggregateType = builder.createStructType();
    auto firstField = builder.createStructKey();
    builder.createStructField(aggregateType, firstField, intType);
    builder.createStructField(aggregateType, builder.createStructKey(), intType);

    IRType* parameterTypes[] = {builder.getPtrType(aggregateType), aggregateType};
    auto func = beginLoadStoreTestFunction(builder, aggregateType, 2, parameterTypes);
    auto address = func->getFirstParam();
    auto initialValue = address->getNextParam();
    auto fieldAddress = builder.emitFieldAddress(address, firstField);
    builder.emitStore(address, initialValue);
    builder.emitStore(fieldAddress, builder.getIntValue(intType, 9));
    auto load = builder.emitLoad(address);
    auto returnInst = builder.emitReturn(load);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(!eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 1);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 2);
    SLANG_CHECK(returnInst->getOperand(0) == load);
}

// Distinct pointer parameters can designate the same location. The second store must prevent
// forwarding through the first parameter even though their IR operands differ.
SLANG_UNIT_TEST(irLoadStoreRetainsLoadAfterPossibleAlias)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    auto pointerType = builder.getPtrType(intType);
    IRType* parameterTypes[] = {pointerType, pointerType};
    auto func = beginLoadStoreTestFunction(builder, intType, 2, parameterTypes);
    auto firstAddress = func->getFirstParam();
    auto secondAddress = firstAddress->getNextParam();
    builder.emitStore(firstAddress, builder.getIntValue(intType, 7));
    builder.emitStore(secondAddress, builder.getIntValue(intType, 9));
    auto load = builder.emitLoad(firstAddress);
    auto returnInst = builder.emitReturn(load);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(!eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 1);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 2);
    SLANG_CHECK(returnInst->getOperand(0) == load);
}

// The store's memory scope must cover the load's scope. We test both directions on otherwise
// identical functions: an invocation-scoped store cannot supply a workgroup-scoped load, while a
// workgroup-scoped store can supply an invocation-scoped load.
SLANG_UNIT_TEST(irLoadStoreRespectsLoadMemoryScope)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto storedValue = builder.getIntValue(intType, 7);
    auto alignment = builder.getIntValue(intType, 4);
    auto invocationScope = builder.getIntValue(intType, IRIntegerValue(MemoryScope::Invocation));
    auto workgroupScope = builder.getIntValue(intType, IRIntegerValue(MemoryScope::Workgroup));

    auto narrowStoreFunc = beginLoadStoreTestFunction(builder, intType, 1, parameterTypes);
    auto narrowStoreAddress = narrowStoreFunc->getFirstParam();
    builder.emitStore(narrowStoreAddress, storedValue, alignment, invocationScope);
    IRInst* workgroupLoadAttrs[] = {builder.getAttr(kIROp_MemoryScopeAttr, workgroupScope)};
    auto workgroupLoad =
        builder.emitLoad(intType, narrowStoreAddress, makeArrayView(workgroupLoadAttrs));
    auto workgroupReturn = builder.emitReturn(workgroupLoad);

    auto wideStoreFunc = beginLoadStoreTestFunction(builder, intType, 1, parameterTypes);
    auto wideStoreAddress = wideStoreFunc->getFirstParam();
    builder.emitStore(wideStoreAddress, storedValue, alignment, workgroupScope);
    IRInst* invocationLoadAttrs[] = {builder.getAttr(kIROp_MemoryScopeAttr, invocationScope)};
    auto invocationLoad =
        builder.emitLoad(intType, wideStoreAddress, makeArrayView(invocationLoadAttrs));
    auto invocationReturn = builder.emitReturn(invocationLoad);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(!eliminateRedundantLoadStore(narrowStoreFunc));
    SLANG_CHECK(eliminateRedundantLoadStore(wideStoreFunc));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(narrowStoreFunc, kIROp_Load) == 1);
    SLANG_CHECK(workgroupReturn->getOperand(0) == workgroupLoad);
    SLANG_CHECK(countLoadStoreTestInsts(wideStoreFunc, kIROp_Load) == 0);
    SLANG_CHECK(invocationReturn->getOperand(0) == storedValue);
}

// Two stores through the same pointer have two distinct operand uses. The first store is
// overwritten and disappears; the final store remains observable through the pointer parameter.
SLANG_UNIT_TEST(irLoadStoreRemovesOverwrittenFirstStore)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto func = beginLoadStoreTestFunction(builder, builder.getVoidType(), 1, parameterTypes);
    auto address = func->getFirstParam();
    builder.emitStore(address, builder.getIntValue(intType, 7));
    auto finalValue = builder.getIntValue(intType, 9);
    auto finalStore = builder.emitStore(address, finalValue);
    builder.emitReturn();

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 1);
    SLANG_CHECK(func->getFirstBlock()->getFirstOrdinaryInst() == finalStore);
}

// A single pointer use does not rule out finding an overwrite: following unconditional branches
// around a cycle reaches the same store again. The pointer parameter dominates the loop body,
// so the loop has one static pointer use even though the store executes repeatedly.
SLANG_UNIT_TEST(irLoadStoreFollowsSingleUsePointerAroundBranchCycle)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto func = beginLoadStoreTestFunction(builder, builder.getVoidType(), 1, parameterTypes);
    auto address = func->getFirstParam();
    auto entryBlock = func->getFirstBlock();
    auto loopBody = builder.emitBlock();
    auto afterLoop = builder.emitBlock();

    // We model `for (;;) { *address = 7; }`. The exit block is present for the loop's structured
    // control-flow metadata but has no incoming execution edge.
    builder.setInsertInto(entryBlock);
    builder.emitLoop(loopBody, afterLoop, loopBody);
    builder.setInsertInto(loopBody);
    auto store = builder.emitStore(address, builder.getIntValue(intType, 7));
    builder.emitBranch(loopBody);
    builder.setInsertInto(afterLoop);
    builder.emitReturn();

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK_ABORT(address->firstUse);
    SLANG_CHECK(address->firstUse->getUser() == store);
    SLANG_CHECK(!address->firstUse->nextUse);
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 0);
}

// An `IRLoop` terminator transfers control unconditionally to its target block. We put an
// immediately exiting inner loop after the store, then return to the outer loop body. Finding
// the same store again requires following the inner `IRLoop` terminator, not just instructions
// with opcode `kIROp_UnconditionalBranch`.
SLANG_UNIT_TEST(irLoadStoreFollowsSingleUsePointerThroughLoopTerminator)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto func = beginLoadStoreTestFunction(builder, builder.getVoidType(), 1, parameterTypes);
    auto address = func->getFirstParam();
    auto entryBlock = func->getFirstBlock();
    auto outerBody = builder.emitBlock();
    auto innerBody = builder.emitBlock();
    auto afterInnerLoop = builder.emitBlock();
    auto afterOuterLoop = builder.emitBlock();

    // Consider `for (;;) { *address = 7; for (;;) { break; } }`. We build the loops directly
    // because CFG simplification can erase the inner loop before an end-to-end test reaches
    // load/store redundancy elimination.
    builder.setInsertInto(entryBlock);
    builder.emitLoop(outerBody, afterOuterLoop, outerBody);
    builder.setInsertInto(outerBody);
    auto store = builder.emitStore(address, builder.getIntValue(intType, 7));
    builder.emitLoop(innerBody, afterInnerLoop, innerBody);
    builder.setInsertInto(innerBody);
    builder.emitBranch(afterInnerLoop);
    builder.setInsertInto(afterInnerLoop);
    builder.emitBranch(outerBody);
    builder.setInsertInto(afterOuterLoop);
    builder.emitReturn();

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK_ABORT(address->firstUse);
    SLANG_CHECK(address->firstUse->getUser() == store);
    SLANG_CHECK(!address->firstUse->nextUse);
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 0);
}

// Storing a loaded value back to the same address is redundant without a later overwrite. We
// return the loaded value so the remaining load is observable independently of the removed store.
SLANG_UNIT_TEST(irLoadStoreRemovesUnchangedValueStoredBack)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    IRType* parameterTypes[] = {builder.getPtrType(intType)};
    auto func = beginLoadStoreTestFunction(builder, intType, 1, parameterTypes);
    auto address = func->getFirstParam();
    auto load = builder.emitLoad(address);
    builder.emitStore(address, load);
    auto returnInst = builder.emitReturn(load);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 0);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 1);
    SLANG_CHECK(returnInst->getOperand(0) == load);
}

// Forwarding a pointer-valued load changes the destination operand of a later store. We must
// index that store's updated operand when visiting the store, so its value can supply the final
// load through the updated pointer.
SLANG_UNIT_TEST(irLoadStoreTracksStoreAddressAfterPointerForwarding)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    auto intType = builder.getIntType();
    auto pointerType = builder.getPtrType(intType);
    IRType* parameterTypes[] = {builder.getPtrType(pointerType), pointerType};
    auto func = beginLoadStoreTestFunction(builder, intType, 2, parameterTypes);
    auto pointerCell = func->getFirstParam();
    auto pointeeAddress = pointerCell->getNextParam();

    // We model `*pointerCell = pointeeAddress; **pointerCell = 7; return **pointerCell;` with a
    // shared load of the pointer. Eliminating that first load rewrites both later address uses.
    builder.emitStore(pointerCell, pointeeAddress);
    auto loadedPointer = builder.emitLoad(pointerCell);
    auto storedValue = builder.getIntValue(intType, 7);
    builder.emitStore(loadedPointer, storedValue);
    auto loadedValue = builder.emitLoad(loadedPointer);
    auto returnInst = builder.emitReturn(loadedValue);

    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(eliminateRedundantLoadStore(func));
    checkLoadStoreTestIR(module.get());
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Load) == 0);
    SLANG_CHECK(countLoadStoreTestInsts(func, kIROp_Store) == 2);
    SLANG_CHECK(returnInst->getOperand(0) == storedValue);
}
