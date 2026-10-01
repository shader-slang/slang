// Direct tests of local record-array roles and linked-IR address provenance.
#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm.h"
#include "slang/slang-ir-legalize-varying-params.h"
#include "slang/slang-ir-nvvm-legalize.h"

using namespace Slang;

namespace
{

// Build canonical padded records once per module; array roles must reuse these exact field keys
// and element types rather than accepting a separately reconstructed structural equivalent.
struct LocalRecordArrayIR
{
    explicit LocalRecordArrayIR(Session* session)
        : module(IRModule::create(session)), builder(module.get())
    {
        builder.setInsertInto(module.get());
        bf16 = builder.getType(kIROp_BFloat16Type);
        pair = builder.getVectorType(bf16, 2);
        payload = builder.createStructType();
        builder.createStructField(payload, builder.createStructKey(), builder.getUInt16Type());
        pairField = builder.createStructField(payload, builder.createStructKey(), pair);
        builder.createStructField(payload, builder.createStructKey(), builder.getUInt16Type());
        outer = builder.createStructType();
        builder.createStructField(outer, builder.createStructKey(), builder.getUInt8Type());
        innerField = builder.createStructField(outer, builder.createStructKey(), payload);
        builder.createStructField(outer, builder.createStructKey(), builder.getUIntType());
        payloadArray = cast<IRArrayType>(builder.getArrayTypeBase(
            kIROp_ArrayType,
            payload,
            builder.getIntValue(builder.getIntType(), 2)));
        outerArray = cast<IRArrayType>(builder.getArrayTypeBase(
            kIROp_ArrayType,
            outer,
            builder.getIntValue(builder.getIntType(), 2)));
    }

    RefPtr<IRModule> module;
    IRBuilder builder;
    IRType* bf16;
    IRVectorType* pair;
    IRStructType* payload;
    IRStructType* outer;
    IRStructField* pairField;
    IRStructField* innerField;
    IRArrayType* payloadArray;
    IRArrayType* outerArray;
};

} // namespace

// The canonical SBT opcode is the sole new pointer producer, with ordinary load flags.
SLANG_UNIT_TEST(nvvmOptixSbtPlansKeepStageAndTypeBoundaries)
{
    enum class Case
    {
        Valid,
        Compute,
        AnyHit,
        Intersection,
        InvalidType,
        EntryParameter
    };
    for (auto testCase :
         {Case::Valid,
          Case::Compute,
          Case::AnyHit,
          Case::Intersection,
          Case::InvalidType,
          Case::EntryParameter})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto record = builder.createStructType();
        auto field =
            builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
        auto group = builder.getConstantBufferType(record, builder.getDefaultBufferLayoutType());
        auto entry = builder.createFunc();
        IRType* parameter = builder.getUIntType();
        const bool hasParameter = testCase == Case::EntryParameter;
        entry->setFullType(builder.getFuncType(
            hasParameter ? 1 : 0,
            hasParameter ? &parameter : nullptr,
            builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(
                testCase == Case::Compute        ? Stage::Compute
                : testCase == Case::AnyHit       ? Stage::AnyHit
                : testCase == Case::Intersection ? Stage::Intersection
                                                 : Stage::RayGeneration),
            toSlice("raygenMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        if (hasParameter)
            builder.emitParam(parameter);
        auto sbt = builder.emitIntrinsicInst(
            testCase == Case::InvalidType ? parameter : group,
            kIROp_GetOptiXSbtDataPtr,
            0,
            nullptr);
        IRInst* load = nullptr;
        if (testCase != Case::InvalidType)
            load = builder.emitLoad(builder.emitFieldAddress(
                builder.getPtrType(builder.getUIntType()),
                sbt,
                field->getKey()));
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = testCase == Case::Valid || testCase == Case::AnyHit;
        if (valid != SLANG_SUCCEEDED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        if (valid)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(
                requirements.emissionPlan.functionNames[0] ==
                (testCase == Case::AnyHit ? "__anyhit__raygenMain" : "__raygen__raygenMain"));
            SLANG_CHECK(requirements.emissionPlan.namedIntrinsics.getCount() == 1);
            SLANG_CHECK(requirements.emissionPlan.namedIntrinsics[0].source == sbt);
            SLANG_CHECK(requirements.emissionPlan.loads.getCount() == 1);
            SLANG_CHECK(requirements.emissionPlan.loads[0].source == load);
            SLANG_CHECK(requirements.emissionPlan.loads[0].flags == SLANG_NVVM_LOAD_FLAG_NONE);
        }
        else
        {
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(toSlice("E52017")) >= 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Original payload types and stage ownership are checked before any optional provider call.
SLANG_UNIT_TEST(nvvmOptixTracePlansKeepPayloadAndStageBoundaries)
{
    enum class Case
    {
        Valid,
        Float4,
        Compute,
        CallbackTrace,
        AnyHitTrace,
        Bool,
        Padding,
        WrongOperand
    };
    for (auto testCase :
         {Case::Valid,
          Case::Float4,
          Case::Compute,
          Case::CallbackTrace,
          Case::AnyHitTrace,
          Case::Bool,
          Case::Padding,
          Case::WrongOperand})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto globals = builder.createStructType();
        builder.addSynthesizedParameterGroupDecoration(globals);
        auto handleType = builder.getType(kIROp_RaytracingAccelerationStructureType);
        auto field = builder.createStructField(globals, builder.createStructKey(), handleType);
        auto global = builder.createGlobalParam(builder.getType(kIROp_ConstantBufferType, globals));
        auto payload = builder.createStructType();
        IRType* leaf = testCase == Case::Float4
                           ? static_cast<IRType*>(builder.getVectorType(builder.getFloatType(), 4))
                       : testCase == Case::Bool ? static_cast<IRType*>(builder.getBoolType())
                                                : builder.getUIntType();
        builder.createStructField(payload, builder.createStructKey(), leaf);
        if (testCase == Case::Padding)
            builder.createStructField(
                payload,
                builder.createStructKey(),
                builder.getVectorType(builder.getFloatType(), 4));
        UInt count = testCase == Case::Float4 ? 4 : 1;
        const bool valid = testCase == Case::Valid || testCase == Case::Float4;
        SLANG_CHECK(
            getNVVMOptixPayloadRegisterCount(payload) ==
            (testCase == Case::Bool || testCase == Case::Padding ? 0 : count));
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(
                testCase == Case::Compute         ? Stage::Compute
                : testCase == Case::CallbackTrace ? Stage::Miss
                : testCase == Case::AnyHitTrace   ? Stage::AnyHit
                                                  : Stage::RayGeneration),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        auto handle =
            builder.emitLoad(handleType, builder.emitFieldAddress(global, field->getKey()));
        List<IRInst*> operands;
        operands.add(payload);
        operands.add(handle);
        for (UInt i = 0; i < 9; ++i)
            operands.add(builder.getFloatValue(builder.getFloatType(), 0));
        for (UInt i = 0; i < 5 + count; ++i)
            operands.add(builder.getIntValue(builder.getUIntType(), i));
        if (testCase == Case::WrongOperand)
            operands[2] = builder.getIntValue(builder.getUIntType(), 0);
        auto array = builder.getArrayTypeBase(
            kIROp_ArrayType,
            builder.getUIntType(),
            builder.getIntValue(builder.getIntType(), count));
        auto trace = builder.emitIntrinsicInst(
            array,
            kIROp_OptixTraceRayPayload,
            operands.getCount(),
            operands.getBuffer());
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        if (valid != SLANG_SUCCEEDED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
        if (valid)
        {
            SLANG_CHECK_ABORT(requirements.emissionPlan.traceRays.getCount() == 1);
            const auto& planned = requirements.emissionPlan.traceRays[0];
            SLANG_CHECK(planned.source == trace && planned.desc.payloadCount == count);
            SLANG_CHECK(planned.operands.getCount() == 15 + count);
            SLANG_CHECK(planned.operands[0] == handle);
            NVVMIRBuilder provider;
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, provider)));
            ComPtr<IArtifact> artifact;
            SLANG_CHECK(SLANG_FAILED(emitNVVMIRFromLinkedIR(
                &context.codeGen,
                linked,
                provider,
                requirements,
                artifact)));
            SLANG_CHECK(!artifact);
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(
                    toSlice("OptiX register trace")) >= 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    for (auto stage :
         {Stage::Compute, Stage::RayGeneration, Stage::Miss, Stage::ClosestHit, Stage::AnyHit})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder
            .addEntryPointDecoration(entry, Profile(stage), toSlice("callback"), toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* index = builder.getIntValue(builder.getIntType(), 0);
        auto word = builder.emitIntrinsicInst(
            builder.getUIntType(),
            kIROp_GetOptiXPayloadRegister,
            1,
            &index);
        IRInst* args[] = {index, word};
        builder.emitIntrinsicInst(builder.getVoidType(), kIROp_SetOptiXPayloadRegister, 2, args);
        const bool hasAttributes = stage == Stage::ClosestHit || stage == Stage::AnyHit;
        IRInst* attributeWords[2] = {};
        IRInst* attributeValues[2] = {};
        if (hasAttributes)
        {
            for (UInt i = 0; i < 2; ++i)
            {
                IRInst* operands[] = {
                    builder.getUIntType(),
                    builder.getIntValue(builder.getIntType(), i)};
                attributeWords[i] = builder.emitIntrinsicInst(
                    builder.getUIntType(),
                    kIROp_GetOptiXHitAttribute,
                    2,
                    operands);
                IRType* resultType = builder.getFloatType();
                if (i == 1)
                    resultType = builder.getIntType();
                attributeValues[i] = builder.emitBitCast(resultType, attributeWords[i]);
            }
        }
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = stage == Stage::Miss || hasAttributes;
        SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
        if (valid)
        {
            SLANG_CHECK(
                requirements.emissionPlan.namedIntrinsics.getCount() == (hasAttributes ? 4 : 2));
            if (hasAttributes)
            {
                UInt conversions = 0;
                for (const auto& planned : requirements.emissionPlan.valueOperations)
                    for (UInt i = 0; i < 2; ++i)
                        if (planned.source == attributeValues[i])
                        {
                            ++conversions;
                            SLANG_CHECK(planned.source->getOperand(0) == attributeWords[i]);
                            SLANG_CHECK(
                                planned.operation.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET);
                            SLANG_CHECK(
                                planned.operation.operandTypes[0].kind ==
                                SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
                            SLANG_CHECK(
                                planned.operation.resultType.kind ==
                                (i == 0 ? SLANG_NVVM_VALUE_TYPE_FLOATING_POINT
                                        : SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER));
                        }
                SLANG_CHECK(conversions == 2);
            }
            SLANG_CHECK(
                requirements.emissionPlan.functionNames[0] ==
                (stage == Stage::Miss     ? "__miss__callback"
                 : stage == Stage::AnyHit ? "__anyhit__callback"
                                          : "__closesthit__callback"));
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
}

// Explicit-layout memory uses checked field keys and scalar payload lanes, never native record GEP.
SLANG_UNIT_TEST(nvvmLayoutPointerFieldsKeepLayoutAndProvenance)
{
    enum class Case
    {
        Valid,
        ForgedRoot,
        WrongLayout,
        WrongAccess,
        WholeRecord,
        Scoped
    };
    for (auto layoutOp :
         {kIROp_Std430BufferLayoutType, kIROp_ScalarBufferLayoutType, kIROp_CBufferLayoutType})
        for (auto testCase :
             {Case::Valid,
              Case::ForgedRoot,
              Case::WrongLayout,
              Case::WrongAccess,
              Case::WholeRecord,
              Case::Scoped})
        {
            _resetDirectNVVMFakes();
            NVVMStaticTestContext context(unitTestContext);
            context.targetProgram->getOptionSet().set(
                CompilerOptionName::EmitCUDAMethod,
                SLANG_EMIT_CUDA_VIA_NVVM);
            context.targetProgram->getTargetReq()->getOptionSet().addCapabilityAtom(
                Slang::CapabilityName::cuda_sm_8_0);
            auto module = IRModule::create(context.env.getSessionImpl());
            IRBuilder builder(module);
            builder.setInsertInto(module);
            auto a = builder.createStructType();
            auto f0 =
                builder.createStructField(a, builder.createStructKey(), builder.getUInt64Type());
            auto f1 =
                builder.createStructField(a, builder.createStructKey(), builder.getUIntType());
            auto b = builder.createStructType();
            auto vectorType = builder.getVectorType(builder.getFloatType(), 3);
            auto vectorField = builder.createStructField(b, builder.createStructKey(), vectorType);
            auto record = builder.createStructType();
            auto aField = builder.createStructField(record, builder.createStructKey(), a);
            auto test1 =
                builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
            auto bField = builder.createStructField(record, builder.createStructKey(), b);
            auto test2 =
                builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
            IRStructField* boolFields[3];
            for (auto& field : boolFields)
                field = builder.createStructField(
                    record,
                    builder.createStructKey(),
                    builder.getBoolType());
            auto layout = builder.getType(layoutOp);
            auto pointer = builder.getPtrType(
                record,
                AccessQualifier::ReadWrite,
                AddressSpace::UserPointer,
                layout);
            IRInst* global = nullptr;
            if (testCase == Case::ForgedRoot)
            {
                global = builder.createGlobalVar(record);
                global->setFullType(pointer);
            }
            IRType* parameters[] = {pointer};
            auto entry = builder.createFunc();
            entry->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
            builder.addEntryPointDecoration(
                entry,
                Profile(Stage::Compute),
                toSlice("computeMain"),
                toSlice("test"));
            builder.setInsertInto(entry);
            builder.emitBlock();
            auto formal = builder.emitParam(pointer);
            IRInst* root = global ? global : formal;
            auto aAddress = builder.emitFieldAddress(root, aField->getKey());
            if (testCase == Case::WrongLayout || testCase == Case::WrongAccess)
                aAddress->setFullType(builder.getPtrType(
                    a,
                    testCase == Case::WrongAccess ? AccessQualifier::Read
                                                  : AccessQualifier::ReadWrite,
                    AddressSpace::UserPointer,
                    testCase == Case::WrongLayout ? builder.getDefaultBufferLayoutType() : layout));
            auto bAddress = builder.emitFieldAddress(root, bField->getKey());
            IRInst* addresses[] = {
                builder.emitFieldAddress(aAddress, f0->getKey()),
                builder.emitFieldAddress(aAddress, f1->getKey()),
                builder.emitFieldAddress(root, test1->getKey()),
                builder.emitFieldAddress(bAddress, vectorField->getKey()),
                builder.emitFieldAddress(root, test2->getKey()),
                builder.emitFieldAddress(root, boolFields[0]->getKey()),
                builder.emitFieldAddress(root, boolFields[1]->getKey()),
                builder.emitFieldAddress(root, boolFields[2]->getKey())};
            for (auto address : addresses)
            {
                auto type = cast<IRPtrTypeBase>(address->getDataType())->getValueType();
                auto loaded = builder.emitLoad(type, address);
                builder.emitStore(address, loaded);
            }
            auto component = builder.emitElementAddress(
                addresses[3],
                builder.getIntValue(builder.getIntType(), 1));
            builder.emitStore(component, builder.getFloatValue(builder.getFloatType(), -6.5));
            if (testCase == Case::WholeRecord)
                builder.emitLoad(a, aAddress);
            if (testCase == Case::Scoped)
                builder.emitStore(
                    addresses[2],
                    builder.getIntValue(builder.getUIntType(), 7),
                    builder.getIntValue(builder.getIntType(), 4),
                    builder.getIntValue(builder.getIntType(), int(MemoryScope::Device)));
            builder.emitReturn();
            LinkedIR linked = {};
            linked.module = module;
            linked.entryPoints.add(entry);
            NVVMOperationRequirements requirements;
            const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
            const auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
            if ((testCase == Case::Valid) != SLANG_SUCCEEDED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    context.sink.outputBuffer.getBuffer());
            if (testCase != Case::Valid)
            {
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(diagnostic.indexOf(toSlice("E52017")) >= 0);
                if (testCase == Case::Scoped)
                {
                    SLANG_CHECK(diagnostic.indexOf(toSlice("scoped memory access")) >= 0);
                    SLANG_CHECK(diagnostic.indexOf(toSlice("requires SM")) < 0);
                }
            }
            else
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
                const bool isC = layoutOp == kIROp_CBufferLayoutType;
                const bool isStd430 = layoutOp == kIROp_Std430BufferLayoutType;
                const uint64_t expectedOffsets[] = {
                    0,
                    8,
                    isC || isStd430 ? 16u : 12u,
                    0,
                    isStd430 ? 48u
                    : isC    ? 32u
                             : 28u,
                    isStd430 ? 52u
                    : isC    ? 36u
                             : 32u,
                    isStd430 ? 56u
                    : isC    ? 37u
                             : 36u,
                    isStd430 ? 60u
                    : isC    ? 38u
                             : 40u};
                const auto& plan = requirements.emissionPlan;
                for (Index i = 0; i < SLANG_COUNT_OF(addresses); ++i)
                {
                    auto field = plan.addresses.findFieldAddress(addresses[i]);
                    SLANG_CHECK_ABORT(field && field->isLayoutStorage);
                    SLANG_CHECK(field->root == formal && field->byteOffset == expectedOffsets[i]);
                    SLANG_CHECK(field->layoutStorage.laneCount == (i == 3 ? 3 : 1));
                }
                auto selectedB = plan.addresses.findFieldAddress(bAddress);
                SLANG_CHECK_ABORT(selectedB && selectedB->isLayoutStorage);
                SLANG_CHECK(selectedB->byteOffset == (isStd430 ? 32 : isC ? 20 : 16));
                SLANG_CHECK(!selectedB->layoutStorage.valueType);
                auto selectedComponent = plan.addresses.findElementAddress(component);
                SLANG_CHECK_ABORT(selectedComponent && selectedComponent->isLayoutStorage);
                SLANG_CHECK(
                    selectedComponent->root == formal && selectedComponent->byteOffset == 4);
                SLANG_CHECK(plan.loads.getCount() == 8 && plan.stores.getCount() == 9);
                for (const auto& load : plan.loads)
                {
                    SLANG_CHECK(load.flags == SLANG_NVVM_LOAD_FLAG_NONE && !load.isScoped);
                    if (load.layoutBoolConversion.operation)
                    {
                        SLANG_CHECK(
                            load.layoutBoolConversion.operation == SLANG_NVVM_VALUE_OP_NOT_EQUAL);
                        SLANG_CHECK(load.layoutStorage.scalarSize == (isC ? 1 : 4));
                        SLANG_CHECK(
                            load.layoutBoolConversion.operandTypes[0].bitWidth == (isC ? 8 : 32));
                    }
                }
                for (const auto& store : plan.stores)
                    if (store.layoutBoolConversion.operation)
                    {
                        SLANG_CHECK(
                            store.layoutBoolConversion.operation ==
                            SLANG_NVVM_VALUE_OP_INTEGER_CONVERT);
                        SLANG_CHECK(
                            store.layoutBoolConversion.resultType.bitWidth == (isC ? 8 : 32));
                    }
            }
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
}

// Canonicalization preserves the actual producer. An address-only bitcast cannot turn a
// same-typed global into an entry parameter or another approved pointer root.
SLANG_UNIT_TEST(nvvmLayoutPointerReinterpretPreservesProducerChecks)
{
    for (auto layoutOp :
         {kIROp_Std430BufferLayoutType, kIROp_ScalarBufferLayoutType, kIROp_CBufferLayoutType})
        for (bool forged : {false, true})
        {
            _resetDirectNVVMFakes();
            NVVMStaticTestContext context(unitTestContext);
            auto module = IRModule::create(context.env.getSessionImpl());
            IRBuilder builder(module);
            builder.setInsertInto(module);
            auto record = builder.createStructType();
            builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
            auto pointer = builder.getPtrType(
                record,
                AccessQualifier::ReadWrite,
                AddressSpace::UserPointer,
                builder.getType(layoutOp));
            auto outputPointer = builder.getPtrType(
                builder.getUInt64Type(),
                AccessQualifier::ReadWrite,
                AddressSpace::UserPointer,
                builder.getDefaultBufferLayoutType());
            IRInst* global = nullptr;
            if (forged)
            {
                global = builder.createGlobalVar(record);
                global->setFullType(pointer);
            }
            IRType* parameterTypes[] = {pointer, outputPointer};
            auto entry = builder.createFunc();
            entry->setFullType(builder.getFuncType(2, parameterTypes, builder.getVoidType()));
            builder.addEntryPointDecoration(
                entry,
                Profile(Stage::Compute),
                toSlice("computeMain"),
                toSlice("test"));
            // Linked entry points carry KeepAlive; this legalization boundary runs DCE.
            builder.addKeepAliveDecoration(entry);
            builder.setInsertInto(entry);
            builder.emitBlock();
            auto formal = builder.emitParam(pointer);
            auto output = builder.emitParam(outputPointer);
            IRInst* operand = forged ? global : formal;
            auto bits = builder.emitBitCast(builder.getUInt64Type(), operand);
            auto store = cast<IRStore>(builder.emitStore(output, bits));
            builder.emitReturn();
            LinkedIR linked = {};
            linked.module = module;
            linked.entryPoints.add(entry);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legalizeIRForNVVM(&context.codeGen, linked)));
            auto observation = store->getVal();
            SLANG_CHECK_ABORT(observation->getOp() == kIROp_CastPtrToInt);
            SLANG_CHECK(observation->getDataType() == builder.getUInt64Type());
            SLANG_CHECK(observation->getOperand(0) == operand);
            NVVMOperationRequirements requirements;
            const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
            if (forged)
            {
                SLANG_CHECK(SLANG_FAILED(result));
                const auto text = context.sink.outputBuffer.getUnownedSlice();
                if (text.indexOf(toSlice("pointer address producer")) < 0)
                    getTestReporter()->message(
                        TestMessageType::Info,
                        context.sink.outputBuffer.getBuffer());
                SLANG_CHECK(text.indexOf(toSlice("pointer address producer")) >= 0);
            }
            else
            {
                if (SLANG_FAILED(result))
                    getTestReporter()->message(
                        TestMessageType::Info,
                        context.sink.outputBuffer.getBuffer());
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
                SLANG_CHECK(
                    requirements.emissionPlan.pointerToIntegerValues[observation] == operand);
            }
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
}

// Collected globals use compact three-lane storage without making their fields writable.
SLANG_UNIT_TEST(nvvmConventionalGlobalVectorsUseCheckedStorage)
{
    enum class Case
    {
        Load,
        Store,
        ForgedRoot
    };
    for (auto elementOp : {kIROp_IntType, kIROp_UIntType, kIROp_FloatType})
    {
        for (uint32_t width : {2u, 3u, 4u})
        {
            for (auto testCase : {Case::Load, Case::Store, Case::ForgedRoot})
            {
                // One representative vector is enough to check the shared permission/root gates.
                if (testCase != Case::Load && (elementOp != kIROp_UIntType || width != 3))
                    continue;
                _resetDirectNVVMFakes();
                NVVMStaticTestContext context(unitTestContext);
                auto module = IRModule::create(context.env.getSessionImpl());
                IRBuilder builder(module);
                builder.setInsertInto(module);
                auto globals = builder.createStructType();
                if (testCase != Case::ForgedRoot)
                    builder.addSynthesizedParameterGroupDecoration(globals);
                builder.createStructField(
                    globals,
                    builder.createStructKey(),
                    builder.getUIntType());
                auto vector = builder.getVectorType(builder.getType(elementOp), width);
                auto field = builder.createStructField(globals, builder.createStructKey(), vector);
                builder.createStructField(
                    globals,
                    builder.createStructKey(),
                    builder.getUIntType());
                auto global =
                    builder.createGlobalParam(builder.getType(kIROp_ConstantBufferType, globals));
                auto entry = builder.createFunc();
                entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
                builder.addEntryPointDecoration(
                    entry,
                    Profile(Stage::Compute),
                    toSlice("computeMain"),
                    toSlice("test"));
                builder.setInsertInto(entry);
                builder.emitBlock();
                auto pointer = builder.getPtrType(
                    vector,
                    AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    builder.getType(kIROp_ScalarBufferLayoutType));
                auto address = builder.emitFieldAddress(pointer, global, field->getKey());
                auto loaded = builder.emitLoad(vector, address);
                if (testCase == Case::Store)
                    builder.emitStore(address, loaded);
                builder.emitReturn();
                LinkedIR linked = {};
                linked.module = module;
                linked.entryPoints.add(entry);
                NVVMOperationRequirements requirements;
                const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
                const auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
                const auto expected = testCase == Case::Store
                                          ? toSlice("immutable struct field access: consumer=store")
                                          : toSlice("global_param");
                if ((testCase == Case::Load && SLANG_FAILED(result)) ||
                    (testCase != Case::Load && diagnostic.indexOf(expected) < 0))
                {
                    getTestReporter()->message(
                        TestMessageType::Info,
                        context.sink.outputBuffer.getBuffer());
                }
                if (testCase == Case::Load)
                {
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
                    const auto selected =
                        requirements.emissionPlan.addresses.findFieldAddress(address);
                    SLANG_CHECK_ABORT(selected);
                    SLANG_CHECK(selected->source == address && selected->base == global);
                    SLANG_CHECK(selected->root == global && selected->selection.field == field);
                    SLANG_CHECK(selected->selection.fieldIndex == 1);
                    SLANG_CHECK(selected->selection.isConventionalGlobal);
                    SLANG_CHECK(!selected->selection.isMutable && !selected->isLayoutStorage);
                    SLANG_CHECK_ABORT(requirements.emissionPlan.loads.getCount() == 1);
                    const auto& load = requirements.emissionPlan.loads[0];
                    SLANG_CHECK(load.source == loaded && load.pointer == address);
                    SLANG_CHECK(load.flags == SLANG_NVVM_LOAD_FLAG_INVARIANT);
                    SLANG_CHECK(load.alignment == (width == 3 ? 4 : width * 4));
                    SLANG_CHECK(
                        load.conversion.kind == (width == 3
                                                     ? NVVMStorageConversionKind::CompactVector
                                                     : NVVMStorageConversionKind::Identity));
                    if (width == 3)
                    {
                        SLANG_CHECK(load.conversion.type == vector);
                        SLANG_CHECK(load.conversion.laneCount == 3);
                    }
                }
                else
                {
                    SLANG_CHECK(SLANG_FAILED(result));
                    SLANG_CHECK(diagnostic.indexOf(toSlice("E52017")) >= 0);
                    SLANG_CHECK(diagnostic.indexOf(expected) >= 0);
                }
                SLANG_CHECK(requirements.emissionPlan.stores.getCount() == 0);
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
            }
        }
    }
}

// A loaded pointer is a root only when checked records prove the direct conventional-cbuffer
// chain. Loads need not be in the first block; unrelated nested group loads stay unqualified.
SLANG_UNIT_TEST(nvvmParameterGroupLayoutPointerLoadsKeepCheckedRoots)
{
    for (bool nested : {false, true})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto record = builder.createStructType();
        builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
        auto pointer = builder.getPtrType(
            record,
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            builder.getType(kIROp_ScalarBufferLayoutType));
        auto fields = builder.createStructType();
        auto pointerField = builder.createStructField(fields, builder.createStructKey(), pointer);
        auto groupType = builder.getType(kIROp_ConstantBufferType, fields);
        IRStructField* nestedField = nullptr;
        IRType* outerGroupType = groupType;
        if (nested)
        {
            auto wrapper = builder.createStructType();
            nestedField = builder.createStructField(wrapper, builder.createStructKey(), groupType);
            outerGroupType = builder.getType(kIROp_ConstantBufferType, wrapper);
        }
        auto globals = builder.createStructType();
        builder.addSynthesizedParameterGroupDecoration(globals);
        auto groupField =
            builder.createStructField(globals, builder.createStructKey(), outerGroupType);
        auto global = builder.createGlobalParam(builder.getType(kIROp_ConstantBufferType, globals));
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        auto first = builder.emitBlock();
        auto body = builder.createBlock();
        builder.emitBranch(body);
        builder.insertBlock(body);
        auto groupAddress = builder.emitFieldAddress(global, groupField->getKey());
        auto group = builder.emitLoad(outerGroupType, groupAddress);
        if (nested)
            group =
                builder.emitLoad(groupType, builder.emitFieldAddress(group, nestedField->getKey()));
        auto fieldAddress = builder.emitFieldAddress(group, pointerField->getKey());
        auto loaded = builder.emitLoad(pointer, fieldAddress);
        auto offset =
            builder.emitGetOffsetPtr(loaded, builder.getIntValue(builder.getIntType(), 1));
        builder.emitIntrinsicInst(builder.getUInt64Type(), kIROp_CastPtrToInt, 1, &offset);
        builder.emitReturn();
        SLANG_CHECK(loaded->getParent() != first);
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const auto text = context.sink.outputBuffer.getUnownedSlice();
        if ((!nested && SLANG_FAILED(result)) ||
            (nested && text.indexOf(toSlice("load result type")) < 0))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        if (nested)
        {
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(text.indexOf(toSlice("load result type")) >= 0);
        }
        else
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            bool sawRoot = false;
            for (const auto& load : requirements.emissionPlan.loads)
                if (load.source == loaded)
                {
                    sawRoot = true;
                    SLANG_CHECK(load.isLayoutPointerRoot);
                    SLANG_CHECK(load.flags == SLANG_NVVM_LOAD_FLAG_INVARIANT);
                }
            SLANG_CHECK(sawRoot);
            SLANG_CHECK(requirements.emissionPlan.layoutPointerOffsets[offset].root == loaded);
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// A valid first call does not establish provenance for later actuals of the same canonical type.
SLANG_UNIT_TEST(nvvmLayoutPointerHelpersCheckEveryCallProducer)
{
    for (auto layoutOp :
         {kIROp_Std430BufferLayoutType, kIROp_ScalarBufferLayoutType, kIROp_CBufferLayoutType})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto record = builder.createStructType();
        builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
        auto pointer = builder.getPtrType(
            record,
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            builder.getType(layoutOp));
        IRType* parameters[] = {pointer};
        auto helper = builder.createFunc();
        helper->setFullType(builder.getFuncType(1, parameters, builder.getUInt64Type()));
        builder.setInsertInto(helper);
        builder.emitBlock();
        IRInst* formal = builder.emitParam(pointer);
        auto address =
            builder.emitIntrinsicInst(builder.getUInt64Type(), kIROp_CastPtrToInt, 1, &formal);
        builder.emitReturn(address);

        builder.setInsertInto(module);
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* actual = builder.emitParam(pointer);
        builder.emitCallInst(builder.getUInt64Type(), helper, 1, &actual);
        auto terminator = builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
        SLANG_CHECK(requirements.emissionPlan.pointerToIntegerValues.containsKey(address));

        builder.setInsertInto(module);
        IRInst* global = builder.createGlobalVar(record);
        global->setFullType(pointer);
        SLANG_CHECK_ABORT(global->getDataType() == helper->getParamType(0));
        builder.setInsertBefore(terminator);
        builder.emitCallInst(builder.getUInt64Type(), helper, 1, &global);
        SLANG_CHECK(SLANG_FAILED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
        const auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
        if (diagnostic.indexOf(toSlice("layout pointer call argument producer")) < 0)
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(diagnostic.indexOf(toSlice("E52017")) >= 0);
        SLANG_CHECK(diagnostic.indexOf(toSlice("layout pointer call argument producer")) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Start with each admitted role in turn. A cached local or internal parameter representation must
// neither authorize a native result/resource role nor be poisoned by its earlier rejection.
SLANG_UNIT_TEST(nvvmLocalRecordArrayTypeRolesIgnoreCacheOrder)
{
    NVVMStaticTestContext context(unitTestContext);
    LocalRecordArrayIR ir(context.env.getSessionImpl());
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    const NVVMTypeUse forbidden[] = {
        NVVMTypeUse::HelperValue,
        NVVMTypeUse::HelperResult,
        NVVMTypeUse::EntryPointParameter,
        NVVMTypeUse::EntryPointResult,
        NVVMTypeUse::ParameterGroupStorage,
        NVVMTypeUse::StructuredBufferStorage,
    };
    const NVVMTypeUse admitted[] = {
        NVVMTypeUse::Value,
        NVVMTypeUse::Storage,
        NVVMTypeUse::HelperParameter};
    IRArrayType* arrays[] = {ir.payloadArray, ir.outerArray};
    for (auto array : arrays)
    {
        for (auto first : admitted)
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &provider;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                provider.createModule(toSlice("local-record-array-cache"), scope.module)));
            NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
            for (auto use : forbidden)
            {
                SlangNVVMTypeHandle rejected = nullptr;
                SLANG_CHECK(SLANG_FAILED(lowering.lowerType(array, use, rejected)));
                SLANG_CHECK(rejected == nullptr);
            }

            if (first == NVVMTypeUse::HelperParameter)
            {
                // Populate the reference representation before any array value/storage lookup.
                for (IROp op :
                     {kIROp_BorrowInParamType, kIROp_OutParamType, kIROp_BorrowInOutParamType})
                {
                    SlangNVVMTypeHandle reference = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(
                        op == kIROp_BorrowInParamType
                            ? ir.builder.getBorrowInParamType(array, AddressSpace::Generic)
                            : ir.builder.getPtrType(op, array),
                        NVVMTypeUse::HelperParameter,
                        reference)));
                    SLANG_CHECK(reference != nullptr);
                }
            }
            SlangNVVMTypeHandle firstType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(array, first, firstType)));
            SLANG_CHECK_ABORT(firstType != nullptr);
            for (auto use : admitted)
            {
                SlangNVVMTypeHandle nextType = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(array, use, nextType)));
                SLANG_CHECK(firstType == nextType);
            }

            // Construct the independent expected LLVM type directly through the provider.
            SlangNVVMTypeHandle i16 = nullptr;
            SlangNVVMTypeHandle bf2 = nullptr;
            SlangNVVMTypeHandle record = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 16, i16)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getVectorType(scope.module, i16, 2, bf2)));
            const SlangNVVMTypeHandle fields[] = {i16, bf2, i16};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                provider.getStructType(scope.module, fields, SLANG_COUNT_OF(fields), record)));
            if (array == ir.outerArray)
            {
                SlangNVVMTypeHandle i8 = nullptr;
                SlangNVVMTypeHandle i32 = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 8, i8)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 32, i32)));
                const SlangNVVMTypeHandle outerFields[] = {i8, record, i32};
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getStructType(
                    scope.module,
                    outerFields,
                    SLANG_COUNT_OF(outerFields),
                    record)));
            }
            SlangNVVMTypeHandle expected = nullptr;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(provider.getArrayType(scope.module, record, 2, expected)));
            SLANG_CHECK(firstType == expected);

            for (auto use : forbidden)
            {
                SlangNVVMTypeHandle rejected = expected;
                SLANG_CHECK(SLANG_FAILED(lowering.lowerType(array, use, rejected)));
                SLANG_CHECK(rejected == nullptr);
            }
            for (auto use : admitted)
            {
                SlangNVVMTypeHandle repeated = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(array, use, repeated)));
                SLANG_CHECK(repeated == expected);
            }
            for (IROp referenceOp :
                 {kIROp_OutParamType, kIROp_BorrowInOutParamType, kIROp_BorrowInParamType})
            {
                auto reference = referenceOp == kIROp_BorrowInParamType
                                     ? ir.builder.getBorrowInParamType(array, AddressSpace::Generic)
                                     : ir.builder.getPtrType(referenceOp, array);
                SlangNVVMTypeHandle expectedPointer = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
                    scope.module,
                    expected,
                    SLANG_NVVM_ADDRESS_SPACE_GENERIC,
                    expectedPointer)));
                for (bool afterAdmission : {false, true})
                {
                    for (auto use :
                         {NVVMTypeUse::Value,
                          NVVMTypeUse::Storage,
                          NVVMTypeUse::HelperValue,
                          NVVMTypeUse::HelperResult,
                          NVVMTypeUse::EntryPointParameter,
                          NVVMTypeUse::EntryPointResult,
                          NVVMTypeUse::ParameterGroupStorage,
                          NVVMTypeUse::StructuredBufferStorage})
                    {
                        SlangNVVMTypeHandle rejected = expectedPointer;
                        SLANG_CHECK(SLANG_FAILED(lowering.lowerType(reference, use, rejected)));
                        SLANG_CHECK(rejected == nullptr);
                    }
                    SlangNVVMTypeHandle admittedReference = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(
                        reference,
                        NVVMTypeUse::HelperParameter,
                        admittedReference)));
                    SLANG_CHECK(admittedReference == expectedPointer);
                    SlangNVVMTypeHandle repeatedArray = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        lowering.lowerType(array, NVVMTypeUse::Storage, repeatedArray)));
                    SLANG_CHECK(repeatedArray == expected);
                    SLANG_UNUSED(afterAdmission);
                }
            }
            auto pointer = ir.builder.getPtrType(kIROp_PtrType, array);
            for (auto use :
                 {NVVMTypeUse::Value, NVVMTypeUse::HelperParameter, NVVMTypeUse::HelperResult})
            {
                SlangNVVMTypeHandle rejected = expected;
                SLANG_CHECK(SLANG_FAILED(lowering.lowerType(pointer, use, rejected)));
                SLANG_CHECK(rejected == nullptr);
            }
        }
    }
}

// The same exact Ptr<Array> spelling on other producers cannot inherit a local Var's address
// permission. Successful preflight also records the complete array→record→BF2 lane chain.
SLANG_UNIT_TEST(nvvmLocalRecordArrayAddressesRequireLocalProducer)
{
    enum class RootKind
    {
        Local,
        InternalOut,
        InternalInOut,
        InternalRead,
        Global,
        EntryParameter,
        BlockParameter,
        Undefined,
    };
    const RootKind kinds[] = {
        RootKind::Local,
        RootKind::InternalOut,
        RootKind::InternalInOut,
        RootKind::InternalRead,
        RootKind::Global,
        RootKind::EntryParameter,
        RootKind::BlockParameter,
        RootKind::Undefined,
    };
    for (auto kind : kinds)
    {
        NVVMStaticTestContext context(unitTestContext);
        LocalRecordArrayIR ir(context.env.getSessionImpl());
        auto& builder = ir.builder;
        const bool isReadOnly = kind == RootKind::InternalRead;
        const bool isInternal =
            kind == RootKind::InternalOut || kind == RootKind::InternalInOut || isReadOnly;
        const bool isSupported = kind == RootKind::Local || isInternal;
        auto pointerType = builder.getPtrType(kIROp_PtrType, ir.outerArray);
        IRInst* root = nullptr;
        if (kind == RootKind::Global)
            root = builder.createGlobalVar(ir.outerArray);
        auto entry = builder.createFunc();
        IRType* parameterTypes[] = {builder.getUIntType(), pointerType};
        const UInt parameterCount = kind == RootKind::EntryParameter ? 2 : 1;
        entry->setFullType(
            builder.getFuncType(parameterCount, parameterTypes, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        auto block = builder.emitBlock();
        auto input = builder.emitParam(builder.getUIntType());
        IRInst* localAllocation = nullptr;
        IRInst* helperCall = nullptr;
        if (isInternal)
        {
            auto referenceType =
                isReadOnly ? builder.getBorrowInParamType(ir.outerArray, AddressSpace::Generic)
                           : builder.getPtrType(
                                 kind == RootKind::InternalOut ? kIROp_OutParamType
                                                               : kIROp_BorrowInOutParamType,
                                 ir.outerArray);
            builder.setInsertInto(ir.module.get());
            auto helper = builder.createFunc();
            IRType* helperTypes[] = {referenceType, builder.getUIntType()};
            helper->setFullType(builder.getFuncType(2, helperTypes, builder.getVoidType()));
            builder.setInsertInto(block);
            localAllocation = builder.emitVar(ir.outerArray);
            IRInst* arguments[] = {localAllocation, input};
            helperCall = builder.emitCallInst(builder.getVoidType(), helper, 2, arguments);
            builder.emitReturn();
            builder.setInsertInto(helper);
            builder.emitBlock();
            root = builder.emitParam(referenceType);
            input = builder.emitParam(builder.getUIntType());
        }
        if (kind == RootKind::EntryParameter)
            root = builder.emitParam(pointerType);
        auto index = builder.emitBitAnd(
            builder.getUIntType(),
            input,
            builder.getIntValue(builder.getUIntType(), 1));
        if (kind == RootKind::Undefined)
            root = builder.emitLoadFromUninitializedMemory(pointerType);
        else if (kind == RootKind::Local || kind == RootKind::BlockParameter)
            root = builder.emitVar(ir.outerArray);
        SLANG_CHECK_ABORT(root != nullptr);
        if (!isInternal)
            SLANG_CHECK_ABORT(root->getDataType() == pointerType);

        if (kind == RootKind::BlockParameter)
        {
            auto next = builder.emitBlock();
            auto parameter = builder.emitParam(pointerType);
            builder.setInsertInto(block);
            builder.emitBranch(next, 1, &root);
            builder.setInsertInto(next);
            root = parameter;
            SLANG_CHECK_ABORT(root->getDataType() == pointerType);
        }

        // Zero-initialize the positive array so whole-value transport is well-defined.
        IRInst* wholeLoad = nullptr;
        IRInst* wholeStore = nullptr;
        IRInst* pairValue = nullptr;
        IRInst* bf16Value = nullptr;
        if (isSupported)
        {
            const auto helperInsertLoc = builder.getInsertLoc();
            if (isReadOnly)
                builder.setInsertBefore(helperCall);
            bf16Value =
                builder.emitBitCast(ir.bf16, builder.getIntValue(builder.getUInt16Type(), 0));
            IRInst* lanes[] = {bf16Value, bf16Value};
            pairValue = builder.emitMakeVector(ir.pair, SLANG_COUNT_OF(lanes), lanes);
            IRInst* payloadFields[] = {
                builder.getIntValue(builder.getUInt16Type(), 0),
                pairValue,
                builder.getIntValue(builder.getUInt16Type(), 0)};
            auto payload =
                builder.emitMakeStruct(ir.payload, SLANG_COUNT_OF(payloadFields), payloadFields);
            IRInst* outerFields[] = {
                builder.getIntValue(builder.getUInt8Type(), 0),
                payload,
                builder.getIntValue(builder.getUIntType(), 0)};
            auto outer = builder.emitMakeStruct(ir.outer, SLANG_COUNT_OF(outerFields), outerFields);
            auto array = builder.emitMakeArrayFromElement(ir.outerArray, outer);
            builder.emitStore(isReadOnly ? localAllocation : root, array);
            builder.setInsertLoc(helperInsertLoc);
            wholeLoad = builder.emitLoad(root);
            auto destination = builder.emitVar(ir.outerArray);
            wholeStore = builder.emitStore(destination, wholeLoad);
        }
        auto element = builder.emitElementAddress(root, index);
        auto inner = builder.emitFieldAddress(element, ir.innerField->getKey());
        auto pair = builder.emitFieldAddress(inner, ir.pairField->getKey());
        auto lane = builder.emitElementAddress(pair, index);
        IRInst* pairStore = nullptr;
        IRInst* laneStore = nullptr;
        IRInst* pairLoad = nullptr;
        IRInst* laneLoad = nullptr;
        if (isReadOnly)
        {
            pairLoad = builder.emitLoad(pair);
            laneLoad = builder.emitLoad(lane);
        }
        else if (isSupported)
        {
            pairStore = builder.emitStore(pair, pairValue);
            laneStore = builder.emitStore(lane, bf16Value);
        }
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = ir.module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        if (!isSupported)
        {
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(requirements.emissionPlan.addresses.findElementAddress(element) == nullptr);
            SLANG_CHECK(requirements.emissionPlan.addresses.findFieldAddress(inner) == nullptr);
            // Parent availability is checked before selecting a child role. This ordinary global
            // is neither admitted shared/parameter storage nor an available local SSA producer.
            if (kind == RootKind::Global)
            {
                SLANG_CHECK(
                    context.sink.outputBuffer.getUnownedSlice().indexOf(toSlice("global_var")) >=
                    0);
            }
            continue;
        }
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        const auto& plan = requirements.emissionPlan;
        SLANG_CHECK_ABORT(plan.localStorage.getCount() == 2);
        SLANG_CHECK(plan.localStorage[0].source == (isInternal ? localAllocation : root));
        SLANG_CHECK(plan.localStorage[0].valueType == ir.outerArray);
        SLANG_CHECK(plan.localStorage[0].valueUse == NVVMTypeUse::Storage);
        SLANG_CHECK(plan.localStorage[0].alignment == 4);
        const auto selected = plan.addresses.findElementAddress(element);
        SLANG_CHECK_ABORT(selected != nullptr);
        SLANG_CHECK(selected->kind == NVVMElementAddressKind::Sequential);
        SLANG_CHECK(selected->base == root && selected->index == index);
        SLANG_CHECK(selected->aggregateType == ir.outerArray);
        SLANG_CHECK(selected->resultType->getValueType() == ir.outer);
        SLANG_CHECK(selected->isReadOnly == isReadOnly && !selected->isParameterGroupStorage);
        const auto innerSelection = plan.addresses.findFieldAddress(inner);
        const auto pairSelection = plan.addresses.findFieldAddress(pair);
        SLANG_CHECK_ABORT(innerSelection && pairSelection);
        SLANG_CHECK(innerSelection->selection.field == ir.innerField);
        SLANG_CHECK(pairSelection->selection.field == ir.pairField);
        SLANG_CHECK(innerSelection->selection.isLocalSubstandardRecordStorage);
        SLANG_CHECK(pairSelection->selection.isLocalSubstandardRecordStorage);
        SLANG_CHECK(innerSelection->selection.isMutable == !isReadOnly);
        SLANG_CHECK(pairSelection->selection.isMutable == !isReadOnly);
        auto laneSelection = plan.addresses.findElementAddress(lane);
        SLANG_CHECK_ABORT(laneSelection);
        SLANG_CHECK(laneSelection->isReadOnly == isReadOnly);
        SLANG_CHECK(plan.addresses.getRoot(lane) == root);
        bool sawWholeLoad = false;
        UInt checkedLoads = 0;
        for (const auto& load : plan.loads)
        {
            if (load.source == wholeLoad || load.source == pairLoad || load.source == laneLoad)
            {
                ++checkedLoads;
                sawWholeLoad |= load.source == wholeLoad;
                SLANG_CHECK(load.flags == SLANG_NVVM_LOAD_FLAG_NONE);
                SLANG_CHECK(load.alignment == (load.source == laneLoad ? 2 : 4));
                SLANG_CHECK(load.conversion.kind == NVVMStorageConversionKind::Identity);
            }
        }
        SLANG_CHECK(sawWholeLoad);
        SLANG_CHECK(checkedLoads == (isReadOnly ? 3 : 1));
        UInt checkedStores = 0;
        for (const auto& store : plan.stores)
        {
            if (store.source == wholeStore || store.source == pairStore ||
                store.source == laneStore)
            {
                ++checkedStores;
                SLANG_CHECK(store.alignment == (store.source == laneStore ? 2 : 4));
                SLANG_CHECK(store.conversion.kind == NVVMStorageConversionKind::Identity);
            }
        }
        SLANG_CHECK(checkedStores == (isReadOnly ? 1 : 3));
    }
    // Type equality is not a producer proof. These calls use the exact formal reference type;
    // a global reaches argument validation, while undefined/block values reject even earlier.
    for (auto kind : {RootKind::Global, RootKind::Undefined, RootKind::BlockParameter})
    {
        NVVMStaticTestContext context(unitTestContext);
        LocalRecordArrayIR ir(context.env.getSessionImpl());
        auto& builder = ir.builder;
        auto reference = builder.getPtrType(kIROp_BorrowInOutParamType, ir.outerArray);
        IRInst* global = builder.createGlobalVar(ir.outerArray);
        global->setFullType(reference);
        auto helper = builder.createFunc();
        IRType* helperTypes[] = {reference};
        helper->setFullType(builder.getFuncType(1, helperTypes, builder.getVoidType()));
        builder.setInsertInto(helper);
        builder.emitBlock();
        builder.emitParam(reference);
        builder.emitReturn();
        builder.setInsertInto(ir.module.get());
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        auto block = builder.emitBlock();
        IRInst* argument = global;
        if (kind == RootKind::Undefined)
            argument = builder.emitLoadFromUninitializedMemory(reference);
        else if (kind == RootKind::BlockParameter)
        {
            auto next = builder.emitBlock();
            argument = builder.emitParam(reference);
            builder.setInsertInto(block);
            builder.emitBranch(next, 1, &global);
            builder.setInsertInto(next);
        }
        SLANG_CHECK_ABORT(argument->getDataType() == helper->getParamType(0));
        builder.emitCallInst(builder.getVoidType(), helper, 1, &argument);
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = ir.module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        SLANG_CHECK(SLANG_FAILED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
        const auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
        const auto expected =
            kind == RootKind::Global ? toSlice("call argument type")
            : kind == RootKind::Undefined
                ? UnownedStringSlice(getIROpInfo(kIROp_LoadFromUninitializedMemory).name)
                : toSlice("basic-block parameter");
        if (diagnostic.indexOf(expected) < 0)
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(diagnostic.indexOf(expected) >= 0);
    }
    // Read access is not write authority, even when the exact array pointee is shared.
    // Build these cases directly because source checking normally rejects both operations.
    for (bool forwardToMutable : {false, true})
    {
        NVVMStaticTestContext context(unitTestContext);
        LocalRecordArrayIR ir(context.env.getSessionImpl());
        auto& builder = ir.builder;
        auto readType = builder.getBorrowInParamType(ir.outerArray, AddressSpace::Generic);
        auto writeType = builder.getPtrType(kIROp_BorrowInOutParamType, ir.outerArray);
        auto writer = builder.createFunc();
        IRType* writerTypes[] = {writeType};
        writer->setFullType(builder.getFuncType(1, writerTypes, builder.getVoidType()));
        builder.setInsertInto(writer);
        builder.emitBlock();
        builder.emitParam(writeType);
        builder.emitReturn();
        builder.setInsertInto(ir.module.get());
        auto reader = builder.createFunc();
        IRType* readerTypes[] = {readType};
        reader->setFullType(builder.getFuncType(1, readerTypes, builder.getVoidType()));
        builder.setInsertInto(reader);
        builder.emitBlock();
        IRInst* borrowed = builder.emitParam(readType);
        if (forwardToMutable)
            builder.emitCallInst(builder.getVoidType(), writer, 1, &borrowed);
        else
        {
            auto element =
                builder.emitElementAddress(borrowed, builder.getIntValue(builder.getUIntType(), 0));
            auto inner = builder.emitFieldAddress(element, ir.innerField->getKey());
            auto pair = builder.emitFieldAddress(inner, ir.pairField->getKey());
            auto value =
                builder.emitBitCast(ir.bf16, builder.getIntValue(builder.getUInt16Type(), 0));
            IRInst* lanes[] = {value, value};
            builder.emitStore(pair, builder.emitMakeVector(ir.pair, 2, lanes));
        }
        builder.emitReturn();
        builder.setInsertInto(ir.module.get());
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* local = builder.emitVar(ir.outerArray);
        builder.emitCallInst(builder.getVoidType(), reader, 1, &local);
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = ir.module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        SLANG_CHECK(SLANG_FAILED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
        const auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
        const auto expected = forwardToMutable ? toSlice("call argument type")
                                               : toSlice("immutable struct field access");
        if (diagnostic.indexOf(expected) < 0)
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(diagnostic.indexOf(expected) >= 0);
        SLANG_CHECK(requirements.emissionPlan.stores.getCount() == 0);
    }
}
