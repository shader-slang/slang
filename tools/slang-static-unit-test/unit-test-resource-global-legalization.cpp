// unit-test-resource-global-legalization.cpp
//
// Tests for resource-global legalization and its shared IR classifications, exercised directly on
// hand-built IR.

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/slang-capability.h"
#include "slang/slang-ir-explicit-global-init.h"
#include "slang/slang-ir-legalize-resource-globals.h"
#include "slang/slang-ir-util.h"
#include "slang/slang-ir-validate.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// These dedicated operations all return values that depend on resource contents. We keep their
// classification in one helper so analyses do not rely on each operation's default side-effect
// classification.
SLANG_UNIT_TEST(resourceContentReadClassificationIncludesDedicatedOperations)
{
    SLANG_CHECK(doesOpReadResourceContents(kIROp_ImageGatherOffset));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_Sample));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_SampleGrad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_StructuredBufferConsume));
}

// Some operations directly produce an address into resource contents. Initializer analysis must
// classify those results as addresses into resource contents instead of following operand zero as
// it does for `GetElementPtr` and pointer-cast results. Loading through such a result reads
// resource contents even when the resource is read-only.
SLANG_UNIT_TEST(resourceContentAddressClassificationIncludesDedicatedRoots)
{
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_GetStructuredBufferPtr));
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_GetUntypedBufferPtr));
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_ImageSubscript));
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_ImageTexelPointer));
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_RWStructuredBufferGetElementPtr));
    SLANG_CHECK(doesOpProduceResourceContentAddress(kIROp_SPIRVLoadTexelPointerFromHeap));
    SLANG_CHECK(!doesOpProduceResourceContentAddress(kIROp_GetElementPtr));
}

// A `BindExistentials` type can have a pointer-like base type even though the resulting type is not
// an `IRPtrTypeBase`. We construct `BindExistentials<Ptr<Interface>, ConcreteType, Witness>`
// directly and verify that initializer validation treats a load through it as a read from
// preexisting mutable storage.
SLANG_UNIT_TEST(resourceInitializerRejectsLoadThroughBoundExistentialPointer)
{
    StaticUnitTestEnv env(unitTestContext);

    // The initializer pass needs the target capabilities used to select applicable target
    // intrinsics. This fixture contains no calls, but obtaining the real target program keeps the
    // pass invocation within its production contract.
    auto targetOwner =
        env.checkModuleFromSource("resourceGlobalBindExistentialsTarget", "void unused() {}");
    SLANG_RELEASE_ASSERT(targetOwner);
    auto linkage = targetOwner->getLinkage();
    SLANG_RELEASE_ASSERT(linkage->targets.getCount() == 1);
    auto targetProgram = targetOwner->getTargetProgram(linkage->targets[0]);

    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    // We use a pointer to an interface as the base type and supply one concrete type and witness
    // table as its existential arguments. `tryGetPointedToType` recognizes the resulting
    // `BindExistentials` type as pointer-like even though it is not an `IRPtrTypeBase`.
    auto interfaceType = builder.createInterfaceType(0, nullptr);
    auto concreteType = builder.createStructType();
    auto witnessTable = builder.createWitnessTable(interfaceType, concreteType);
    auto basePointerType = builder.getPtrType(interfaceType, AddressSpace::Generic);
    IRInst* existentialArguments[] = {concreteType, witnessTable};
    auto boundPointerType = builder.getBindExistentialsType(
        basePointerType,
        SLANG_COUNT_OF(existentialArguments),
        existentialArguments);
    SLANG_RELEASE_ASSERT(as<IRBindExistentialsType>(boundPointerType));
    auto loadedType = tryGetPointedToType(&builder, boundPointerType);
    SLANG_RELEASE_ASSERT(loadedType);

    auto externalAddress = builder.createGlobalParam(boundPointerType);
    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeStaticVarDecoration);

    // The initializer loads through `externalAddress`, whose type is the `BindExistentials` type
    // constructed above, and returns a separate resource input. The unused load is intentional:
    // the read itself makes moving the initializer unsafe.
    builder.setInsertInto(resourceGlobal);
    builder.emitBlock();
    builder.emitLoad(loadedType, externalAddress);
    builder.emitReturn(resourceInput);

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);

    DiagnosticSink sink;
    moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
        module.get(),
        targetProgram,
        &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56013]")) >= 0);

    // Initializer safety validation completes before rewriting, so a rejected initializer remains
    // attached to its global.
    SLANG_CHECK(resourceGlobal->getFirstBlock());
}

// A resource global marked for replacement can be referenced by metadata attached to another
// module object. No function owns such a reference, so the pass has no local address with which to
// replace it. We build that module directly because source expressions that inspect only a value's
// type are folded before this pass and therefore cannot preserve the module-scope reference we need
// to test. We reject the decoration instead of silently deleting metadata owned by another object.
SLANG_UNIT_TEST(resourceGlobalLegalizationRejectsReferenceOutsideFunctionBody)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeStaticVarDecoration);

    auto metadataOwner = builder.createGlobalParam(builder.getFloatType());
    builder.addNameHintDecoration(metadataOwner, UnownedStringSlice("metadataOwner"));
    auto externalReference =
        builder.addDecoration(metadataOwner, kIROp_DependsOnDecoration, resourceGlobal);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56016]")) >= 0);

    // Validation happens before rewriting. We therefore retain both the resource global marked for
    // replacement and the metadata edge when reporting the unsupported reference.
    SLANG_CHECK(resourceGlobal->getParent() == module->getModuleInst());
    SLANG_CHECK(externalReference->getParent() == metadataOwner);
    SLANG_CHECK(externalReference->getOperand(0) == resourceGlobal);
}

// A function-level decoration is not part of the function body, so it cannot reference a local
// declared in the entry block. A function declaration has no block in which the pass could create
// a local. We diagnose both references before rewriting either function.
SLANG_UNIT_TEST(resourceGlobalLegalizationRejectsFunctionLevelReference)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeStaticVarDecoration);

    auto voidFuncType = builder.getFuncType(0, nullptr, builder.getVoidType());
    auto definedFunc = builder.createFunc();
    definedFunc->setFullType(voidFuncType);
    builder.setInsertInto(definedFunc);
    builder.emitBlock();
    builder.emitReturn();
    auto definedFuncReference =
        builder.addDecoration(definedFunc, kIROp_DependsOnDecoration, resourceGlobal);

    builder.setInsertInto(module.get());
    auto declaredFunc = builder.createFunc();
    declaredFunc->setFullType(voidFuncType);
    auto declaredFuncReference =
        builder.addDecoration(declaredFunc, kIROp_DependsOnDecoration, resourceGlobal);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), &sink);

    SLANG_CHECK(sink.getErrorCount() == 2);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56016]")) >= 0);

    // Validation happens before rewriting. Both decorations must still refer to the original
    // global, and the function definition must not gain a replacement local.
    SLANG_CHECK(definedFuncReference->getOperand(0) == resourceGlobal);
    SLANG_CHECK(declaredFuncReference->getOperand(0) == resourceGlobal);
    SLANG_CHECK(
        definedFunc->getFirstBlock()->getFirstOrdinaryInst() ==
        definedFunc->getFirstBlock()->getTerminator());
}

// A directional parameter describes source-level access, but its IR body can contain an explicit
// pointer cast that performs an additional effect. We build the essential lowered shape directly:
// a `BorrowIn` parameter is written, and another function passes the selected global to it. The
// caller must receive a generated `inout` parameter so that the write returns to its caller.
SLANG_UNIT_TEST(resourceGlobalLegalizationAccountsForWritesThroughBorrowInParameters)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeStaticVarDecoration);

    // `writeThroughBorrowIn` models a source `__constref` parameter followed by a cast that removes
    // the read-only wrapper. The store is deliberately inconsistent with the wrapper because that
    // inconsistency is the case the pass must discover in the IR body.
    auto borrowInType = builder.getBorrowInParamType(resourceType, AddressSpace::Generic);
    IRType* writerParamTypes[] = {borrowInType};
    auto writerType = builder.getFuncType(1, writerParamTypes, builder.getVoidType());
    auto writer = builder.createFunc();
    writer->setFullType(writerType);
    builder.setInsertInto(writer);
    builder.emitBlock();
    auto writerParam = builder.emitParam(borrowInType);
    auto writablePointerType = builder.getPtrType(resourceType, AddressSpace::Generic);
    IRInst* castOperands[] = {writerParam};
    auto writableAddress =
        builder.emitIntrinsicInst(writablePointerType, kIROp_PtrCast, 1, castOperands);
    builder.emitStore(writableAddress, resourceInput);
    builder.emitReturn();

    // `callWriter` accesses the global only by passing its address. Correct access classification
    // must still discover the write in `writeThroughBorrowIn` and give `callWriter` a generated
    // `inout` parameter.
    builder.setInsertInto(module.get());
    auto voidFunctionType = builder.getFuncType(0, nullptr, builder.getVoidType());
    auto callWriter = builder.createFunc();
    callWriter->setFullType(voidFunctionType);
    builder.setInsertInto(callWriter);
    builder.emitBlock();
    IRInst* writerArgs[] = {resourceGlobal};
    builder.emitCallInst(builder.getVoidType(), writer, 1, writerArgs);
    builder.emitReturn();

    // The entry point initializes the global before calling `callWriter`, so the later
    // uninitialized-value check has a complete initial value to follow.
    builder.setInsertInto(module.get());
    auto entryPoint = builder.createFunc();
    entryPoint->setFullType(voidFunctionType);
    builder.addEntryPointDecoration(
        entryPoint,
        Profile(Stage::Compute),
        UnownedStringSlice("main"),
        UnownedStringSlice("test"));
    builder.setInsertInto(entryPoint);
    builder.emitBlock();
    builder.emitStore(resourceGlobal, resourceInput);
    builder.emitCallInst(builder.getVoidType(), callWriter, 0, nullptr);
    builder.emitLoad(resourceType, resourceGlobal);
    builder.emitReturn();

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(callWriter->getParamCount() == 1);
    SLANG_CHECK(as<IRBorrowInOutParamType>(callWriter->getFirstParam()->getDataType()));

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);
}

// A recursive call cycle has no callee-first ordering, so the resource-access analysis must reach a
// fixed point. We construct two mutually recursive helpers, give only the first helper a direct
// read of the selected global, and call only the second helper from the entry point. Legalization
// must add a read-only resource parameter to both helpers and rewrite all three calls.
SLANG_UNIT_TEST(resourceGlobalLegalizationPropagatesReadsThroughRecursiveCallCycle)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeStaticVarDecoration);

    auto voidFunctionType = builder.getFuncType(0, nullptr, builder.getVoidType());
    auto firstHelper = builder.createFunc();
    firstHelper->setFullType(voidFunctionType);
    auto secondHelper = builder.createFunc();
    secondHelper->setFullType(voidFunctionType);

    // `firstHelper` is the only function with a direct resource read. Its call to `secondHelper`
    // closes the recursive cycle.
    builder.setInsertInto(firstHelper);
    builder.emitBlock();
    builder.emitLoad(resourceType, resourceGlobal);
    builder.emitCallInst(builder.getVoidType(), secondHelper, 0, nullptr);
    builder.emitReturn();

    builder.setInsertInto(secondHelper);
    builder.emitBlock();
    builder.emitCallInst(builder.getVoidType(), firstHelper, 0, nullptr);
    builder.emitReturn();

    // The entry point supplies the initial resource value and enters the cycle through the helper
    // that has no direct global use.
    builder.setInsertInto(module.get());
    auto entryPoint = builder.createFunc();
    entryPoint->setFullType(voidFunctionType);
    builder.addEntryPointDecoration(
        entryPoint,
        Profile(Stage::Compute),
        UnownedStringSlice("main"),
        UnownedStringSlice("test"));
    builder.setInsertInto(entryPoint);
    builder.emitBlock();
    builder.emitStore(resourceGlobal, resourceInput);
    builder.emitCallInst(builder.getVoidType(), secondHelper, 0, nullptr);
    builder.emitReturn();

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(firstHelper->getParamCount() == 1);
    SLANG_CHECK(secondHelper->getParamCount() == 1);
    SLANG_CHECK(firstHelper->getFirstParam()->getDataType() == resourceType);
    SLANG_CHECK(secondHelper->getFirstParam()->getDataType() == resourceType);

    // Each original call is rebuilt after its callee gains a parameter. Finding the calls in the
    // rewritten bodies avoids retaining pointers to the deallocated original call instructions.
    auto findCall = [](IRFunc* caller, IRFunc* callee) -> IRCall*
    {
        for (auto block : caller->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto call = as<IRCall>(inst))
                {
                    if (call->getCallee() == callee)
                        return call;
                }
            }
        }
        return nullptr;
    };

    auto firstToSecondCall = findCall(firstHelper, secondHelper);
    auto secondToFirstCall = findCall(secondHelper, firstHelper);
    auto entryToSecondCall = findCall(entryPoint, secondHelper);
    SLANG_CHECK(firstToSecondCall && firstToSecondCall->getArgCount() == 1);
    SLANG_CHECK(secondToFirstCall && secondToFirstCall->getArgCount() == 1);
    SLANG_CHECK(entryToSecondCall && entryToSecondCall->getArgCount() == 1);

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);
}
