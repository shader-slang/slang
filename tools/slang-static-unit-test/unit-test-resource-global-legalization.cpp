// unit-test-resource-global-legalization.cpp
//
// These tests exercise resource-global legalization and its shared IR classifications on
// hand-built IR. We first test the opcode classifications used by function-property propagation and
// initializer-movement safety analysis. We then test the legalizer's rejection rules, address
// analysis, type handling, and call-graph fixed points.

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/slang-capability.h"
#include "slang/slang-ir-explicit-global-init.h"
#include "slang/slang-ir-fix-entrypoint-callsite.h"
#include "slang/slang-ir-legalize-resource-globals.h"
#include "slang/slang-ir-propagate-func-properties.h"
#include "slang/slang-ir-util.h"
#include "slang/slang-ir-validate.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Find the first direct call from `caller` to `callee`, or return null when no such call exists.
static IRCall* findFirstDirectCall(IRFunc* caller, IRFunc* callee)
{
    // Call rewriting deallocates the original instruction, so tests must rediscover a rebuilt call
    // by traversing every block in its caller.
    for (auto block : caller->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (call->getCallee() == callee)
                return call;
        }
    }
    return nullptr;
}

// ## Shared IR classifications

// These operations all return values that depend on resource contents.
// `doesOpReadResourceContents` keeps that classification independent of each operation's default
// side-effect classification.
SLANG_UNIT_TEST(resourceContentReadClassificationIncludesDedicatedOperations)
{
    SLANG_CHECK(doesOpReadResourceContents(kIROp_ImageGatherOffset));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_ImageLoad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_Sample));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_SampleGrad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_StructuredBufferLoad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_ByteAddressBufferLoad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_StructuredBufferLoadStatus));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_RWStructuredBufferLoad));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_RWStructuredBufferLoadStatus));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_StructuredBufferConsume));
    SLANG_CHECK(doesOpReadResourceContents(kIROp_SubpassLoad));
    SLANG_CHECK(!doesOpReadResourceContents(kIROp_StructuredBufferGetDimensions));
}

// `ReadNone` promises that a function does not read memory, while `NoSideEffect` permits dead-code
// elimination to remove an unused-result call. We put a dedicated `Sample` operation in one
// function and only debug instructions in another. The resource read prevents `ReadNone`. The debug
// instructions prevent both decorations because dead-code elimination has no separate rule that
// preserves the debug records belonging to a removed call. Including `DebugNoScope` covers the
// instruction that terminates a lexical debug scope without starting another one.
SLANG_UNIT_TEST(funcPropertyPropagationRejectsResourceReadsAndDebugInstructions)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto debugSource = builder.emitDebugSource(toSlice("test.slang"), UnownedStringSlice(), false);
    auto voidFunctionType = builder.getFuncType(0, nullptr, builder.getVoidType());

    auto zero = builder.getIntValue(builder.getIntType(), 0);
    auto float2Type = builder.getVectorType(builder.getFloatType(), 2);
    auto float4Type = builder.getVectorType(builder.getFloatType(), 4);
    IRInst* textureTypeOperands[] = {
        float4Type,
        builder.getType(kIROp_TextureShape2DType),
        zero, // isArray
        zero, // isMultisample
        zero, // sampleCount
        zero, // read-only access
        zero, // isShadow
        zero, // isCombined
        zero, // unknown image format
    };
    auto textureType = builder.getType(
        kIROp_TextureType,
        SLANG_COUNT_OF(textureTypeOperands),
        textureTypeOperands);
    auto texture = builder.createGlobalParam(textureType);
    auto sampler = builder.createGlobalParam(cast<IRType>(builder.getType(kIROp_SamplerStateType)));
    auto coordinate = builder.createGlobalParam(float2Type);

    auto resourceReader = builder.createFunc();
    resourceReader->setFullType(voidFunctionType);
    builder.setInsertInto(resourceReader);
    builder.emitBlock();
    builder.emitDebugLine(debugSource, 1, 1, 1, 1);
    IRInst* sampleOperands[] = {texture, sampler, coordinate};
    builder.emitIntrinsicInst(
        float4Type,
        kIROp_Sample,
        SLANG_COUNT_OF(sampleOperands),
        sampleOperands);
    builder.emitReturn();

    builder.setInsertInto(module.get());
    auto debugOnly = builder.createFunc();
    debugOnly->setFullType(voidFunctionType);
    builder.setInsertInto(debugOnly);
    builder.emitBlock();
    builder.emitDebugLine(debugSource, 2, 2, 1, 1);
    builder.emitDebugNoScope();
    builder.emitReturn();

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);

    propagateFuncProperties(module.get());

    SLANG_CHECK(!resourceReader->findDecoration<IRReadNoneDecoration>());
    SLANG_CHECK(!debugOnly->findDecoration<IRReadNoneDecoration>());
    SLANG_CHECK(!debugOnly->findDecoration<IRNoSideEffectDecoration>());
}

// Some operations directly produce an address into resource contents. The initializer-movement
// safety analysis uses `doesOpProduceResourceContentAddress` to recognize those roots instead of
// following operand zero as it does for `GetElementPtr` and pointer-cast results. Loading through
// such a result reads resource contents even when the resource is read-only.
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

// ## Resource-global legalization

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
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    auto metadataOwner = builder.createGlobalParam(builder.getFloatType());
    builder.addNameHintDecoration(metadataOwner, UnownedStringSlice("metadataOwner"));
    auto externalReference =
        builder.addDecoration(metadataOwner, kIROp_DependsOnDecoration, resourceGlobal);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56016]")) >= 0);

    // Validation happens before rewriting. We therefore retain both the resource global marked for
    // replacement and the metadata edge when reporting the unsupported reference.
    SLANG_CHECK(resourceGlobal->getParent() == module->getModuleInst());
    SLANG_CHECK(externalReference->getParent() == metadataOwner);
    SLANG_CHECK(externalReference->getOperand(0) == resourceGlobal);
}

// Removing a global recursively removes all metadata attached to it. That metadata can have an
// external user even when it does not refer to its owner, so inspecting only the global's own use
// list is insufficient. We attach one decoration to the selected global and make metadata on a
// different object refer to that decoration. The pass must reject the external reference before
// removing either the global or its decoration.
SLANG_UNIT_TEST(resourceGlobalLegalizationRejectsExternalUseOfAttachedMetadata)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);
    auto attachedMetadata =
        builder.addDecoration(resourceGlobal, kIROp_DependsOnDecoration, resourceInput);

    auto metadataOwner = builder.createGlobalParam(builder.getFloatType());
    auto externalReference =
        builder.addDecoration(metadataOwner, kIROp_DependsOnDecoration, attachedMetadata);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56018]")) >= 0);
    SLANG_CHECK(resourceGlobal->getParent() == module->getModuleInst());
    SLANG_CHECK(attachedMetadata->getParent() == resourceGlobal);
    SLANG_CHECK(externalReference->getOperand(0) == attachedMetadata);
}

// An external user of attached metadata can also occur inside a function body. The user's location
// does not make recursive deletion safe: removing the selected global would still delete the
// metadata instruction that supplies the user's operand. We attach the external reference to a
// return instruction to verify that the diagnostic describes metadata ownership instead of
// incorrectly claiming that every external user is outside a function body.
SLANG_UNIT_TEST(resourceGlobalLegalizationRejectsBodyUseOfAttachedMetadata)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);
    auto attachedMetadata =
        builder.addDecoration(resourceGlobal, kIROp_DependsOnDecoration, resourceInput);

    auto function = builder.createFunc();
    function->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
    builder.setInsertInto(function);
    builder.emitBlock();
    auto returnInst = builder.emitReturn();
    auto externalReference =
        builder.addDecoration(returnInst, kIROp_DependsOnDecoration, attachedMetadata);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56018]")) >= 0);
    SLANG_CHECK(resourceGlobal->getParent() == module->getModuleInst());
    SLANG_CHECK(attachedMetadata->getParent() == resourceGlobal);
    SLANG_CHECK(externalReference->getOperand(0) == attachedMetadata);
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
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

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
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 2);
    auto diagnostics = sink.outputBuffer.getUnownedSlice();
    auto diagnosticCode = toSlice("error[E56016]");
    auto firstDiagnosticIndex = diagnostics.indexOf(diagnosticCode);
    SLANG_CHECK_ABORT(firstDiagnosticIndex >= 0);
    auto diagnosticsAfterFirst =
        diagnostics.tail(firstDiagnosticIndex + diagnosticCode.getLength());
    SLANG_CHECK(diagnosticsAfterFirst.indexOf(diagnosticCode) >= 0);

    // Validation happens before rewriting. Both decorations must still refer to the original
    // global, and the function definition must not gain a replacement local.
    SLANG_CHECK(definedFuncReference->getOperand(0) == resourceGlobal);
    SLANG_CHECK(declaredFuncReference->getOperand(0) == resourceGlobal);
    SLANG_CHECK(
        definedFunc->getFirstBlock()->getFirstOrdinaryInst() ==
        definedFunc->getFirstBlock()->getTerminator());
}

// A function can appear as an argument as well as the callee of an `IRCall`. The entry-point
// call-site pass must redirect only callee uses; otherwise a call such as `consume(kernel)` would
// incorrectly invoke a clone of `kernel` while retaining `kernel` as its argument. We build that
// shape with a CUDA kernel that accesses a selected resource global. The call-site pass must leave
// the argument use unchanged. Resource-global legalization must then diagnose that function-value
// use because it could lead to an indirect invocation that the pass cannot find or rewrite.
SLANG_UNIT_TEST(entryPointCallsiteFixDoesNotRewriteCallArguments)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    auto voidFunctionType = builder.getFuncType(0, nullptr, builder.getVoidType());
    auto kernel = builder.createFunc();
    kernel->setFullType(voidFunctionType);
    builder.addCudaKernelDecoration(kernel);
    builder.setInsertInto(kernel);
    builder.emitBlock();
    builder.emitStore(resourceGlobal, resourceInput);
    builder.emitLoad(resourceType, resourceGlobal);
    builder.emitReturn();

    // `consume` gives the kernel reference a valid function-typed parameter. Its body does not
    // invoke the parameter because this test needs only the call-argument use that the call-site
    // pass previously mistook for a callee use.
    builder.setInsertInto(module.get());
    IRType* consumerParameterTypes[] = {voidFunctionType};
    auto consumerType = builder.getFuncType(1, consumerParameterTypes, builder.getVoidType());
    auto consumer = builder.createFunc();
    consumer->setFullType(consumerType);
    builder.setInsertInto(consumer);
    builder.emitBlock();
    builder.emitParam(voidFunctionType);
    builder.emitReturn();

    builder.setInsertInto(module.get());
    auto caller = builder.createFunc();
    caller->setFullType(voidFunctionType);
    builder.setInsertInto(caller);
    builder.emitBlock();
    IRInst* consumerArguments[] = {kernel};
    auto consumerCall = builder.emitCallInst(builder.getVoidType(), consumer, 1, consumerArguments);
    builder.emitReturn();

    fixEntryPointCallsites(module.get());

    SLANG_CHECK(consumerCall->getCallee() == consumer);
    SLANG_CHECK(consumerCall->getArg(0) == kernel);

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56010]")) >= 0);
    SLANG_CHECK(consumerCall->getCallee() == consumer);
    SLANG_CHECK(consumerCall->getArg(0) == kernel);
}

// Slang permits an explicit cast to remove the read-only restriction from a `__constref` parameter.
// After validation removes the temporary `AssumeAddress` instruction, the IR contains a store
// directly through a `BorrowIn` parameter. `tests/bugs/13074-static-resource-constref-write.slang`
// covers that producer path. We build the same IR shape directly to check the pass's
// interprocedural effect classification: the caller needs an `inout` parameter so the write returns
// to its caller.
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
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    // `writer` models the lowered body of `replaceThroughConstRef` from the regression test named
    // above. Its store demonstrates why the declared parameter direction alone does not describe
    // the body's effects.
    auto borrowInType = builder.getBorrowInParamType(resourceType, AddressSpace::Generic);
    IRType* writerParamTypes[] = {borrowInType};
    auto writerType = builder.getFuncType(1, writerParamTypes, builder.getVoidType());
    auto writer = builder.createFunc();
    writer->setFullType(writerType);
    builder.setInsertInto(writer);
    builder.emitBlock();
    auto writerParam = builder.emitParam(borrowInType);
    builder.emitStore(writerParam, resourceInput);
    builder.emitReturn();

    // `callWriter` accesses the global only by passing its address. Correct access classification
    // must still discover the write in `writer` and give `callWriter` a generated `inout`
    // parameter.
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
    // uninitialized-value check has a whole initial value to follow.
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
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(callWriter->getParamCount() == 1);
    SLANG_CHECK(as<IRBorrowInOutParamType>(callWriter->getFirstParam()->getDataType()));

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);
}

// Generic assembly supplies a function implementation that does not expose its parameter uses as
// ordinary IR. Passing a selected global's address to such a function therefore prevents the pass
// from proving that the address remains local to the call. We build a helper whose generic-assembly
// body receives the global through an `inout` parameter. The pass must reject the call before it
// replaces the module-scope address with an entry-point local.
SLANG_UNIT_TEST(resourceGlobalLegalizationRejectsAddressPassedToGenericAssembly)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    // The parameter has no ordinary IR user. Because `IRGenericAsm` replaces the analyzed body
    // during emission, that absence does not prove that the emitted implementation ignores the
    // parameter.
    auto inOutType = builder.getBorrowInOutParamType(resourceType);
    IRType* assemblyParamTypes[] = {inOutType};
    auto assemblyFunctionType = builder.getFuncType(1, assemblyParamTypes, builder.getVoidType());
    auto assemblyFunction = builder.createFunc();
    assemblyFunction->setFullType(assemblyFunctionType);
    builder.setInsertInto(assemblyFunction);
    builder.emitBlock();
    builder.emitParam(inOutType);
    builder.emitGenericAsm(toSlice("/* target implementation */"));

    // The entry point initializes the selected global and passes its address to the opaque
    // implementation. A later read makes the resource value observable.
    builder.setInsertInto(module.get());
    auto entryPoint = builder.createFunc();
    entryPoint->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
    builder.addEntryPointDecoration(
        entryPoint,
        Profile(Stage::Compute),
        UnownedStringSlice("main"),
        UnownedStringSlice("test"));
    builder.setInsertInto(entryPoint);
    builder.emitBlock();
    builder.emitStore(resourceGlobal, resourceInput);
    IRInst* arguments[] = {resourceGlobal};
    auto call = builder.emitCallInst(builder.getVoidType(), assemblyFunction, 1, arguments);
    builder.emitLoad(resourceType, resourceGlobal);
    builder.emitReturn();

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 1);
    SLANG_CHECK(sink.outputBuffer.getUnownedSlice().indexOf(toSlice("error[E56009]")) >= 0);
    SLANG_CHECK(resourceGlobal->getParent() == module->getModuleInst());
    SLANG_CHECK(call->getArg(0) == resourceGlobal);
}

// An `IRAttributedType` can wrap a pointer type when an IR transformation preserves type
// attributes. The initializer mover, candidate predicate, and storage-transfer analysis must all
// unwrap that representation. We give an initialized global an attributed pointer type and pass
// its address through a pointer cast, so the test exercises the shared representation contract
// from initializer extraction through resource-global replacement.
SLANG_UNIT_TEST(resourceGlobalLegalizationAcceptsAttributedPointerStorageTransfer)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    auto resourcePointerType = cast<IRType>(resourceGlobal->IRInst::getDataType());
    IRAttr* globalPointerAttributes[] = {builder.getAttr(kIROp_NoDiffAttr)};
    auto attributedGlobalPointerType =
        builder.getAttributedType(resourcePointerType, 1, globalPointerAttributes);
    resourceGlobal->setFullType(attributedGlobalPointerType);

    builder.setInsertInto(resourceGlobal);
    builder.emitBlock();
    builder.emitReturn(resourceInput);

    auto entryPoint = builder.createFunc();
    entryPoint->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
    builder.addEntryPointDecoration(
        entryPoint,
        Profile(Stage::Compute),
        UnownedStringSlice("main"),
        UnownedStringSlice("test"));
    builder.setInsertInto(entryPoint);
    builder.emitBlock();
    auto storageAddress = builder.emitBitCast(resourcePointerType, resourceGlobal);
    auto store = builder.emitStore(storageAddress, resourceInput);
    auto load = builder.emitLoad(resourceType, storageAddress);
    builder.emitReturn();

    auto targetModule = env.checkModuleFromSource("resourceGlobalInitializerTarget", "");
    SLANG_CHECK(targetModule);
    auto linkage = targetModule->getLinkage();
    SLANG_CHECK(linkage->targets.getCount() == 1);
    auto targetProgram = targetModule->getTargetProgram(linkage->targets[0]);

    DiagnosticSink sink;
    moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
        module.get(),
        targetProgram,
        &sink);
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(resourceGlobal->getParent() == nullptr);
    SLANG_CHECK(store->getOperand(0) != resourceGlobal);
    SLANG_CHECK(load->getOperand(0) == store->getOperand(0));

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);
}

// `generatePyTorchCppBinding` lowers an `IRDispatchKernel` to `kIROp_CudaKernelLaunch` before
// resource-global legalization runs for the PyTorch target. Both instructions launch the original
// kernel rather than calling the ordinary-function clone that receives generated arguments.
// Resource-global legalization inserts the replacement local into that original kernel's body, so
// each launch executes with a distinct dynamic instance. We build the lowered form directly because
// it has no source syntax at this stage of the pipeline. Legalization must preserve the launch and
// replace the kernel's use of the global without adding a parameter to the kernel or its host
// function.
SLANG_UNIT_TEST(resourceGlobalLegalizationAcceptsLoweredCudaKernelLaunch)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module.get());
    builder.setInsertInto(module.get());

    auto resourceType = cast<IRType>(builder.getType(kIROp_SamplerStateType));
    auto resourceInput = builder.createGlobalParam(resourceType);
    auto resourceGlobal = builder.createGlobalVar(resourceType);
    builder.addNameHintDecoration(resourceGlobal, UnownedStringSlice("resourceGlobal"));
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

    // The kernel assigns a complete value before reading it, so the test isolates launch-use
    // classification from uninitialized-value diagnostics.
    auto voidFunctionType = builder.getFuncType(0, nullptr, builder.getVoidType());
    auto kernel = builder.createFunc();
    kernel->setFullType(voidFunctionType);
    builder.addCudaKernelDecoration(kernel);
    builder.setInsertInto(kernel);
    builder.emitBlock();
    builder.emitStore(resourceGlobal, resourceInput);
    builder.emitLoad(resourceType, resourceGlobal);
    builder.emitReturn();

    // The host-side launch names the kernel in operand zero. Its remaining operands supply the grid
    // and block dimensions, the argument-array pointer, and the CUDA stream. None carries the
    // implicit mutable resource value that legalization places in the kernel body.
    builder.setInsertInto(module.get());
    auto hostFunction = builder.createFunc();
    hostFunction->setFullType(voidFunctionType);
    builder.setInsertInto(hostFunction);
    builder.emitBlock();
    auto uintType = builder.getUIntType();
    auto uint3Type = builder.getVectorType(uintType, 3);
    auto one = builder.getIntValue(uintType, 1);
    IRInst* dimensionElements[] = {one, one, one};
    auto dimension =
        builder.emitMakeVector(uint3Type, SLANG_COUNT_OF(dimensionElements), dimensionElements);
    auto launch = builder.emitCudaKernelLaunch(
        kernel,
        dimension,
        dimension,
        builder.getNullVoidPtrValue(),
        builder.getNullVoidPtrValue());
    builder.emitReturn();

    DiagnosticSink sink;
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(resourceGlobal->getParent() == nullptr);
    SLANG_CHECK(kernel->getParamCount() == 0);
    SLANG_CHECK(hostFunction->getParamCount() == 0);
    SLANG_CHECK(launch->getOperand(0) == kernel);

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
    builder.addDecoration(resourceGlobal, kIROp_FileOrNamespaceScopeMutableVarDecoration);

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
    legalizeResourceGlobalVars(module.get(), CapabilitySet::makeEmpty(), true, &sink);

    SLANG_CHECK(sink.getErrorCount() == 0);
    SLANG_CHECK(firstHelper->getParamCount() == 1);
    SLANG_CHECK(secondHelper->getParamCount() == 1);
    SLANG_CHECK(firstHelper->getFirstParam()->getDataType() == resourceType);
    SLANG_CHECK(secondHelper->getFirstParam()->getDataType() == resourceType);

    // Each original call is rebuilt after its callee gains a parameter. We rediscover the calls in
    // the rewritten bodies instead of retaining pointers to deallocated instructions.
    auto firstToSecondCall = findFirstDirectCall(firstHelper, secondHelper);
    auto secondToFirstCall = findFirstDirectCall(secondHelper, firstHelper);
    auto entryToSecondCall = findFirstDirectCall(entryPoint, secondHelper);
    SLANG_CHECK_ABORT(firstToSecondCall);
    SLANG_CHECK_ABORT(secondToFirstCall);
    SLANG_CHECK_ABORT(entryToSecondCall);
    SLANG_CHECK(firstToSecondCall->getArgCount() == 1);
    SLANG_CHECK(secondToFirstCall->getArgCount() == 1);
    SLANG_CHECK(entryToSecondCall->getArgCount() == 1);

    DiagnosticSink validationSink;
    validateIRModule(module.get(), &validationSink);
    SLANG_CHECK(validationSink.getErrorCount() == 0);
}
