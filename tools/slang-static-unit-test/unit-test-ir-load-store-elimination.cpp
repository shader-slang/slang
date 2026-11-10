// Tests for load/store redundancy elimination on hand-built IR.
//
// These cases check that skipping impossible searches preserves the pass's elimination decisions.
// We first check load forwarding, covering successful replacements and the writes or memory
// scopes that require keeping a load. We then check store removal, including unconditional
// cycles and storing back an unchanged value. The final case checks that pointer forwarding
// does not prevent subsequent load forwarding.
//
// To isolate these decisions, we build IR and call `eliminateRedundantLoadStore` directly;
// earlier optimizations could otherwise remove the address uses or control flow under test.
// We use pointer parameters so the pass cannot remove all stores through its separate check
// for unused local variables, which could let a test pass without exercising its intended rule.

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

// We exercise a successful forwarding case so the search filter cannot silently disable load
// replacement. We inspect the return operand as well as the load count because deleting the
// load without substituting the stored value must not satisfy the test.
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

// We also exercise forwarding through an array element inside a struct field so the pass must
// handle a computed pointer as well as a parameter. We require both address operations to be
// present before the pass so the test actually exercises forwarding through that projection.
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
    // reference.
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

// To check that the pass rejects stale values after a partial write, we build a whole-value
// store followed by a field store. We require the final load to remain because replacing it
// with the initial whole value would lose the field update.
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

// We cannot assume that distinct pointer parameters refer to disjoint storage. We therefore
// place a write through the second pointer between the first write and its load, and require
// the pass to keep the load rather than reuse a value that may have been overwritten.
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

// We cannot assume that the value visible at workgroup scope was updated by an invocation-scoped
// write. We check that the pass requires the store's scope to cover the load's, using opposite
// scope orderings: the pass must keep a workgroup-scoped load after an invocation-scoped store,
// but can forward in the reverse case.
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

// To exercise a successful overwrite search, we write twice through the same pointer. We require
// the pass to remove the first write and keep the last because the caller can still observe
// that final value through the pointer parameter.
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

// We need the overwrite search to follow a cycle even when the destination has only one static
// use. We therefore put a single store in an infinite loop and require its removal: following
// the back edge lets the pass find that same instruction as an overwrite.
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

    // We model `for (;;) { *address = 7; }`. We create a break-target block because
    // `IRBuilder::emitLoop` requires one, even though the body never branches to it.
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

// We need the overwrite search to follow `IRLoop` as well as ordinary unconditional branches.
// We therefore put an immediately exiting inner loop after the store. The search must follow
// that loop's terminator to reach the store again through the outer loop and remove it.
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

// To check the store-back rule independently of overwrite removal, we provide no later store.
// We return the loaded value to make that load independently needed, then require the pass
// to remove only the redundant store.
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

// Earlier forwarding can change a store's pointer before the traversal records it. We exercise
// that dependency by loading a pointer from a pointer cell, storing through the loaded pointer,
// and then loading the stored value. The pass must forward the pointer load first and record the
// updated store address so the final load can be forwarded too.
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

    // We reuse one pointer load for both later addresses so a single replacement must update
    // both the store and the load before the traversal reaches them.
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
