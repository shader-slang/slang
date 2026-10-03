// Direct tests of local record-array roles and linked-IR address provenance.
#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm.h"
#include "slang/slang-ir-legalize-varying-params.h"
#include "slang/slang-ir-nvvm-legalize.h"
#include "slang/slang-ir-nvvm-surface-legalize.h"
#include "slang/slang-ir-specialize-address-space.h"
#include "slang/slang-ir-use-uninitialized-values.h"

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

// Noncopyable initialization has a destination operand, not a fictitious returned SSA handle.
SLANG_UNIT_TEST(irOpaqueConstructorsWriteOnlyTheirDestination)
{
    enum class Case
    {
        HitObjectAllocation,
        RayQueryAllocation,
        Nop,
        Miss,
        MissUninitialized,
        Traverse,
        TraverseUninitialized
    };
    for (auto testCase :
         {Case::HitObjectAllocation,
          Case::RayQueryAllocation,
          Case::Nop,
          Case::Miss,
          Case::MissUninitialized,
          Case::Traverse,
          Case::TraverseUninitialized})
    {
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        IRType* objectType =
            testCase == Case::RayQueryAllocation
                ? builder.getType(kIROp_RayQueryType, builder.getIntValue(builder.getIntType(), 0))
                : builder.getType(kIROp_HitObjectType);
        auto referenceType = builder.getOutParamType(objectType);
        auto sceneType = builder.getType(kIROp_RaytracingAccelerationStructureType);
        IRType* parameters[] = {referenceType, sceneType};
        auto function = builder.createFunc();
        function->setFullType(builder.getFuncType(2, parameters, builder.getVoidType()));
        builder.setInsertInto(function);
        builder.emitBlock();
        auto destination = builder.emitParam(referenceType);
        auto scene = builder.emitParam(sceneType);
        auto zero = builder.getIntValue(builder.getUIntType(), 0);
        auto floatZero = builder.getFloatValue(builder.getFloatType(), 0);
        List<IRInst*> operands;
        operands.add(destination);
        IROp op = kIROp_AllocateOpaqueHandle;
        IRType* result = builder.getVoidType();
        if (testCase == Case::Nop)
            op = kIROp_OptixHitObjectMakeNop;
        if (testCase == Case::Miss || testCase == Case::MissUninitialized)
        {
            op = kIROp_OptixHitObjectMakeMiss;
            operands.add(zero);
            for (UInt i = 0; i < 9; ++i)
                operands.add(
                    i == 0 && testCase == Case::MissUninitialized
                        ? static_cast<IRInst*>(
                              builder.emitLoadFromUninitializedMemory(builder.getFloatType()))
                        : floatZero);
            operands.add(zero);
        }
        if (testCase == Case::Traverse || testCase == Case::TraverseUninitialized)
        {
            op = kIROp_OptixHitObjectTraverse;
            result = builder.getUIntType();
            operands.add(scene);
            for (UInt i = 0; i < 5; ++i)
                operands.add(zero);
            IRInst* lanes[] = {floatZero, floatZero, floatZero};
            auto vector =
                builder.emitMakeVector(builder.getVectorType(builder.getFloatType(), 3), 3, lanes);
            operands.add(vector);
            operands.add(floatZero);
            operands.add(vector);
            operands.add(floatZero);
            operands.add(floatZero);
            operands.add(
                testCase == Case::TraverseUninitialized
                    ? static_cast<IRInst*>(
                          builder.emitLoadFromUninitializedMemory(builder.getUIntType()))
                    : zero);
        }
        auto constructor =
            builder.emitIntrinsicInst(result, op, operands.getCount(), operands.getBuffer());
        SLANG_CHECK(constructor->mightHaveSideEffects());
        builder.emitReturn();
        checkForUsingUninitializedValues(module, &context.sink);
        const bool bad =
            testCase == Case::MissUninitialized || testCase == Case::TraverseUninitialized;
        SLANG_CHECK(
            bad == String(context.sink.outputBuffer.getUnownedSlice()).contains("uninitialized"));
    }
}

// Object references keep their caller-owned storage identity; provider-private state is not
// an external buffer, entry parameter or numeric value ABI.
SLANG_UNIT_TEST(nvvmOptixHitObjectsKeepOwnedReferences)
{
    enum class Case
    {
        Queries,
        Array,
        Helper,
        GlobalAfterValidCall,
        WrongResult,
        WrongIndex,
        DynamicIndex,
        ReadOnlyConstruction,
        Compute,
        AnyHit,
        Intersection,
        Callable,
        Traverse,
        Invoke,
        EmptyInvoke,
        Report,
        WrongReportStage,
        InvalidPayload
    };
    for (auto testCase :
         {Case::Queries,
          Case::Array,
          Case::Helper,
          Case::GlobalAfterValidCall,
          Case::WrongResult,
          Case::WrongIndex,
          Case::DynamicIndex,
          Case::ReadOnlyConstruction,
          Case::Compute,
          Case::AnyHit,
          Case::Intersection,
          Case::Callable,
          Case::Traverse,
          Case::Invoke,
          Case::EmptyInvoke,
          Case::Report,
          Case::WrongReportStage,
          Case::InvalidPayload})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto objectType = builder.getType(kIROp_HitObjectType);
        auto uintType = builder.getUIntType();
        auto empty = testCase == Case::EmptyInvoke ? builder.createStructType() : nullptr;
        auto arrayType =
            builder.getArrayType(objectType, builder.getIntValue(builder.getIntType(), 2));
        const auto objectInfo = classifyNVVMType(objectType);
        SLANG_CHECK(objectInfo.supports(NVVMTypeUse::Storage));
        for (auto use :
             {NVVMTypeUse::EntryPointParameter,
              NVVMTypeUse::HelperParameter,
              NVVMTypeUse::HelperResult,
              NVVMTypeUse::ParameterGroupStorage,
              NVVMTypeUse::StructuredBufferStorage})
            SLANG_CHECK(!objectInfo.supports(use));
        auto globalObject =
            testCase == Case::GlobalAfterValidCall ? builder.createGlobalVar(objectType) : nullptr;
        IRFunc* helper = nullptr;
        IRType* referenceType = builder.getOutParamType(objectType);
        if (testCase == Case::ReadOnlyConstruction)
            referenceType = builder.getPtrType(
                kIROp_BorrowInParamType,
                objectType,
                AccessQualifier::Read,
                AddressSpace::Generic,
                builder.getDefaultBufferLayoutType());
        if (testCase == Case::Helper || testCase == Case::GlobalAfterValidCall ||
            testCase == Case::ReadOnlyConstruction)
        {
            helper = builder.createFunc();
            IRType* parameters[] = {referenceType};
            helper->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
            builder.setInsertInto(helper);
            builder.emitBlock();
            auto reference = builder.emitParam(referenceType);
            IRInst* args[] = {
                reference,
                builder.getIntValue(uintType, 0),
                builder.getIntValue(uintType, 0)};
            if (testCase == Case::ReadOnlyConstruction)
                builder
                    .emitIntrinsicInst(builder.getVoidType(), kIROp_OptixHitObjectMakeNop, 1, args);
            else
                builder.emitIntrinsicInst(uintType, kIROp_OptixHitObjectQuery, 3, args);
            builder.emitReturn();
            builder.setInsertInto(module);
        }
        IRInst* scene = nullptr;
        IRStructField* sceneField = nullptr;
        if (testCase == Case::Traverse)
        {
            auto globals = builder.createStructType();
            builder.addSynthesizedParameterGroupDecoration(globals);
            sceneField = builder.createStructField(
                globals,
                builder.createStructKey(),
                builder.getType(kIROp_RaytracingAccelerationStructureType));
            scene = builder.createGlobalParam(builder.getType(kIROp_ConstantBufferType, globals));
        }
        const Stage stage = testCase == Case::Compute  ? Stage::Compute
                            : testCase == Case::AnyHit ? Stage::AnyHit
                            : testCase == Case::Intersection || testCase == Case::Report
                                ? Stage::Intersection
                            : testCase == Case::Callable         ? Stage::Callable
                            : testCase == Case::WrongReportStage ? Stage::ClosestHit
                                                                 : Stage::RayGeneration;
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(entry, Profile(stage), toSlice("probe"), toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        const bool report = testCase == Case::Report || testCase == Case::WrongReportStage;
        IRInst* object = nullptr;
        if (!report)
        {
            object = builder.emitVar(testCase == Case::Array ? arrayType : objectType);
            if (testCase == Case::Array)
                object = builder.emitElementAddress(
                    builder.getPtrType(objectType),
                    object,
                    builder.getIntValue(builder.getIntType(), 1));
            auto initial = builder.emitIntrinsicInst(
                builder.getVoidType(),
                kIROp_AllocateOpaqueHandle,
                1,
                &object);
            SLANG_CHECK(initial->mightHaveSideEffects());
        }
        if (helper)
        {
            IRInst* args[] = {object};
            builder.emitCallInst(builder.getVoidType(), helper, 1, args);
            if (globalObject)
            {
                args[0] = globalObject;
                builder.emitCallInst(builder.getVoidType(), helper, 1, args);
            }
        }
        else if (
            testCase == Case::Traverse || testCase == Case::Invoke ||
            testCase == Case::EmptyInvoke || testCase == Case::InvalidPayload || report)
        {
            IRType* payload = testCase == Case::EmptyInvoke ? static_cast<IRType*>(empty)
                              : testCase == Case::InvalidPayload
                                  ? static_cast<IRType*>(builder.getHalfType())
                                  : static_cast<IRType*>(uintType);
            List<IRInst*> args;
            args.add(payload);
            args.add(builder.getIntValue(
                uintType,
                report                       ? 10
                : testCase == Case::Traverse ? 3
                                             : 4));
            if (report)
            {
                args.add(builder.getFloatValue(builder.getFloatType(), 1));
                args.add(builder.getIntValue(uintType, 19));
            }
            else
                args.add(object);
            if (testCase == Case::Traverse)
            {
                args.add(builder.emitLoad(builder.emitFieldAddress(
                    builder.getPtrType(sceneField->getFieldType()),
                    scene,
                    sceneField->getKey())));
                for (UInt i = 0; i < 9; ++i)
                    args.add(builder.getFloatValue(builder.getFloatType(), float(i)));
                for (UInt i = 0; i < 5; ++i)
                    args.add(builder.getIntValue(uintType, i));
            }
            if (testCase != Case::EmptyInvoke)
                args.add(builder.getIntValue(uintType, 7));
            IRType* result =
                report ? static_cast<IRType*>(uintType)
                : testCase == Case::EmptyInvoke
                    ? static_cast<IRType*>(builder.getVoidType())
                    : builder.getArrayType(uintType, builder.getIntValue(builder.getIntType(), 1));
            builder.emitIntrinsicInst(
                result,
                kIROp_OptixHitObjectPayload,
                args.getCount(),
                args.getBuffer());
        }
        else
        {
            const UInt queryCount = testCase == Case::Queries ? 23 : 1;
            for (UInt query = 0; query < queryCount; ++query)
            {
                UInt selectedQuery = testCase == Case::WrongIndex ? 15 : query;
                IRInst* index = builder.getIntValue(uintType, testCase == Case::WrongIndex ? 8 : 0);
                if (testCase == Case::DynamicIndex)
                    index = builder.emitAdd(uintType, index, index);
                IRType* result = selectedQuery >= 10 && selectedQuery <= 14
                                     ? static_cast<IRType*>(builder.getFloatType())
                                     : uintType;
                if (selectedQuery == 10 || selectedQuery == 11)
                    result = builder.getVectorType(builder.getFloatType(), 3);
                if (selectedQuery >= 17 && selectedQuery <= 20)
                    result = builder.getVectorType(builder.getFloatType(), 4);
                if (testCase == Case::WrongResult)
                    result = builder.getUInt64Type();
                IRInst* args[] = {object, builder.getIntValue(uintType, selectedQuery), index};
                auto queryInst =
                    builder.emitIntrinsicInst(result, kIROp_OptixHitObjectQuery, 3, args);
                SLANG_CHECK(queryInst->mightHaveSideEffects());
                SLANG_CHECK(!getIROpInfo(queryInst->getOp()).isHoistable());
            }
        }
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = testCase == Case::Queries || testCase == Case::Array ||
                           testCase == Case::Helper || testCase == Case::Traverse ||
                           testCase == Case::Invoke || testCase == Case::EmptyInvoke ||
                           testCase == Case::Report;
        if (valid != SLANG_SUCCEEDED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
        if (valid)
        {
            const UInt expected = testCase == Case::Queries ? 24 : testCase == Case::Report ? 1 : 2;
            SLANG_CHECK(requirements.emissionPlan.hitObjectOperations.getCount() == expected);
            if (testCase == Case::Queries)
                for (UInt i = 0; i < 23; ++i)
                {
                    const auto& selected = requirements.emissionPlan.hitObjectOperations[i + 1];
                    SLANG_CHECK(selected.desc.query == i);
                    SLANG_CHECK(selected.operands.getCount() == 1);
                    SLANG_CHECK(selected.operands[0] == object);
                }
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
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Variadic reports become the same dense record contract as portable ReportHit attributes.
SLANG_UNIT_TEST(nvvmOptixReportsKeepAttributeAndStageBounds)
{
    enum class Case
    {
        Counts,
        Oversize,
        Boolean,
        WrongStage
    };
    for (auto testCase : {Case::Counts, Case::Oversize, Case::Boolean, Case::WrongStage})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(testCase == Case::WrongStage ? Stage::ClosestHit : Stage::Intersection),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        auto block = builder.emitBlock();
        const UInt first = testCase == Case::Counts ? 0 : testCase == Case::Oversize ? 9 : 1;
        const UInt last = testCase == Case::Counts ? 8 : first;
        for (UInt count = first; count <= last; ++count)
        {
            builder.setInsertInto(module);
            auto attributes = builder.createStructType();
            for (UInt i = 0; i < count; ++i)
            {
                IRType* fieldType = testCase == Case::Boolean
                                        ? static_cast<IRType*>(builder.getBoolType())
                                    : i % 3 == 0 ? static_cast<IRType*>(builder.getUIntType())
                                    : i % 3 == 1 ? static_cast<IRType*>(builder.getIntType())
                                                 : static_cast<IRType*>(builder.getFloatType());
                builder.createStructField(attributes, builder.createStructKey(), fieldType);
            }
            builder.setInsertInto(block);
            List<IRInst*> operands;
            operands.add(attributes);
            operands.add(builder.getIntValue(
                builder.getUIntType(),
                SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION));
            operands.add(builder.getFloatValue(builder.getFloatType(), 1));
            operands.add(builder.getIntValue(builder.getUIntType(), 19));
            for (UInt i = 0; i < count; ++i)
                operands.add(builder.getIntValue(builder.getUIntType(), i + 11));
            builder.emitIntrinsicInst(
                builder.getUIntType(),
                kIROp_OptixHitObjectPayload,
                operands.getCount(),
                operands.getBuffer());
        }
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = testCase == Case::Counts;
        SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
        if (valid)
        {
            SLANG_CHECK_ABORT(requirements.emissionPlan.hitObjectOperations.getCount() == 9);
            for (UInt count = 0; count <= 8; ++count)
            {
                const auto& selected = requirements.emissionPlan.hitObjectOperations[count];
                SLANG_CHECK(
                    selected.desc.operation == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION);
                SLANG_CHECK(selected.desc.payloadCount == count);
                SLANG_CHECK(selected.operands.getCount() == count + 2);
                for (UInt i = 0; i < count; ++i)
                    SLANG_CHECK(
                        as<IRIntLit>(selected.operands[i + 2])->getValue() ==
                        IRIntegerValue(i + 11));
            }
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
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

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
        const bool valid =
            testCase == Case::Valid || testCase == Case::AnyHit || testCase == Case::Intersection;
        if (valid != SLANG_SUCCEEDED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        if (valid)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(
                requirements.emissionPlan.functionNames[0] ==
                (testCase == Case::AnyHit         ? "__anyhit__raygenMain"
                 : testCase == Case::Intersection ? "__intersection__raygenMain"
                                                  : "__raygen__raygenMain"));
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

// The row read is typed, effectful and stage-scoped before provider discovery or mutation.
SLANG_UNIT_TEST(nvvmOptixInstanceRowsRequireCheckedImmediates)
{
    enum class Case
    {
        Valid,
        RowRange,
        SignedRow,
        DynamicRow,
        WrongInverse,
        DynamicInverse,
        WrongHandle,
        WrongResult,
        WrongArity
    };
    for (bool current : {false, true})
        for (auto stage :
             {Stage::RayGeneration,
              Stage::Miss,
              Stage::ClosestHit,
              Stage::AnyHit,
              Stage::Compute,
              Stage::Intersection,
              Stage::Callable})
            for (auto testCase :
                 {Case::Valid,
                  Case::RowRange,
                  Case::SignedRow,
                  Case::DynamicRow,
                  Case::WrongInverse,
                  Case::DynamicInverse,
                  Case::WrongHandle,
                  Case::WrongResult,
                  Case::WrongArity})
            {
                // Type/immediate boundaries do not need to be multiplied by every valid stage.
                if ((stage != (current ? Stage::ClosestHit : Stage::RayGeneration) &&
                     testCase != Case::Valid) ||
                    (current && testCase == Case::WrongHandle))
                    continue;
                _resetDirectNVVMFakes();
                NVVMStaticTestContext context(unitTestContext);
                auto module = IRModule::create(context.env.getSessionImpl());
                IRBuilder builder(module);
                builder.setInsertInto(module);
                auto entry = builder.createFunc();
                entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
                builder.addEntryPointDecoration(
                    entry,
                    Profile(stage),
                    toSlice("probe"),
                    toSlice("test"));
                builder.setInsertInto(entry);
                builder.emitBlock();
                IRInst* handle = builder.getIntValue(builder.getUInt64Type(), 1);
                List<IRInst*> rows;
                for (UInt inverse = 0; inverse < 2; ++inverse)
                    for (UInt row = 0; row < 3; ++row)
                    {
                        IRInst* selectedRow = builder.getIntValue(builder.getUIntType(), row);
                        IRInst* selectedInverse = builder.getBoolValue(inverse != 0);
                        IRType* resultType = builder.getVectorType(builder.getFloatType(), 4);
                        if (testCase == Case::RowRange)
                            selectedRow = builder.getIntValue(builder.getUIntType(), 3);
                        if (testCase == Case::SignedRow)
                            selectedRow = builder.getIntValue(builder.getIntType(), row);
                        if (testCase == Case::DynamicRow)
                            selectedRow =
                                builder.emitAdd(builder.getUIntType(), selectedRow, selectedRow);
                        if (testCase == Case::WrongInverse)
                            selectedInverse = builder.getIntValue(builder.getUIntType(), inverse);
                        if (testCase == Case::DynamicInverse)
                            selectedInverse = builder.emitIntrinsicInst(
                                builder.getBoolType(),
                                kIROp_Not,
                                1,
                                &selectedInverse);
                        if (testCase == Case::WrongHandle)
                            handle = builder.getIntValue(builder.getInt64Type(), 1);
                        if (testCase == Case::WrongResult)
                            resultType = builder.getVectorType(builder.getFloatType(), 3);
                        IRInst* operands[] = {handle, selectedRow, selectedInverse};
                        auto value = builder.emitIntrinsicInst(
                            resultType,
                            current ? kIROp_OptixCurrentTransformRow
                                    : kIROp_OptixInstanceTransformRow,
                            (current ? 2 : 3) - (testCase == Case::WrongArity ? 1 : 0),
                            operands + (current ? 1 : 0));
                        SLANG_CHECK(value->mightHaveSideEffects());
                        SLANG_CHECK(!getIROpInfo(value->getOp()).isHoistable());
                        SLANG_CHECK(value->getParent() == entry->getFirstBlock());
                        rows.add(value);
                    }
                builder.emitReturn();
                LinkedIR linked = {};
                linked.module = module;
                linked.entryPoints.add(entry);
                NVVMOperationRequirements requirements;
                const bool valid =
                    testCase == Case::Valid &&
                    ((!current && (stage == Stage::RayGeneration || stage == Stage::Miss ||
                                   stage == Stage::Callable)) ||
                     stage == Stage::ClosestHit || stage == Stage::AnyHit ||
                     stage == Stage::Intersection);
                auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
                if (valid != SLANG_SUCCEEDED(result))
                    getTestReporter()->message(
                        TestMessageType::Info,
                        context.sink.outputBuffer.getBuffer());
                SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
                if (valid)
                {
                    SLANG_CHECK_ABORT(requirements.emissionPlan.instanceTransforms.getCount() == 6);
                    NVVMEmissionPlanIndex index;
                    index.initialize(requirements.emissionPlan);
                    for (Index i = 0; i < 6; ++i)
                    {
                        auto planned = index.findInstanceTransform(rows[i]);
                        SLANG_CHECK_ABORT(planned);
                        SLANG_CHECK(
                            planned->source == rows[i] &&
                            planned->handle == (current ? nullptr : handle));
                        SLANG_CHECK(planned->desc.row == uint32_t(i % 3));
                        SLANG_CHECK(planned->desc.inverse == uint32_t(i / 3));
                    }
                    NVVMIRBuilder provider;
                    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, provider)));
                    ComPtr<IArtifact> artifact;
                    SLANG_CHECK(SLANG_FAILED(emitNVVMIRFromLinkedIR(
                        &context.codeGen,
                        linked,
                        provider,
                        requirements,
                        artifact)));
                    SLANG_CHECK(!artifact);
                }
                const auto expectedDiagnostic = toSlice("OptiX transform row");
                SLANG_CHECK(
                    context.sink.outputBuffer.getUnownedSlice().indexOf(expectedDiagnostic) >= 0);
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
        FloatArray12,
        Float3Array,
        NestedMixed32,
        ZeroArray,
        NegativeArray,
        OversizeArray,
        HugeArray,
        NonliteralArray,
        UnsizedArray,
        NaturalStrideArray,
        InflatedStrideArray,
        PaddedRecordArray,
        BoolArray,
        HalfArray,
        UInt64Array,
        Compute,
        MissTrace,
        ClosestHitTrace,
        IntersectionTrace,
        DirectCallableTrace,
        AnyHitTrace,
        Bool,
        Padding,
        WrongOperand
    };
    for (auto testCase :
         {Case::Valid,
          Case::Float4,
          Case::FloatArray12,
          Case::Float3Array,
          Case::NestedMixed32,
          Case::ZeroArray,
          Case::NegativeArray,
          Case::OversizeArray,
          Case::HugeArray,
          Case::NonliteralArray,
          Case::UnsizedArray,
          Case::NaturalStrideArray,
          Case::InflatedStrideArray,
          Case::PaddedRecordArray,
          Case::BoolArray,
          Case::HalfArray,
          Case::UInt64Array,
          Case::Compute,
          Case::MissTrace,
          Case::ClosestHitTrace,
          Case::IntersectionTrace,
          Case::DirectCallableTrace,
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
        IRType* leaf = builder.getUIntType();
        UInt count = 1;
        bool payloadValid = true;
        switch (testCase)
        {
        case Case::Float4:
            leaf = builder.getVectorType(builder.getFloatType(), 4);
            count = 4;
            break;
        case Case::Bool:
            leaf = builder.getBoolType();
            break;
        case Case::Padding:
            count = 8;
            break;
        case Case::FloatArray12:
        case Case::Float3Array:
        case Case::NestedMixed32:
        case Case::ZeroArray:
        case Case::NegativeArray:
        case Case::OversizeArray:
        case Case::HugeArray:
        case Case::NonliteralArray:
        case Case::UnsizedArray:
        case Case::NaturalStrideArray:
        case Case::InflatedStrideArray:
        case Case::PaddedRecordArray:
        case Case::BoolArray:
        case Case::HalfArray:
        case Case::UInt64Array:
            {
                IRType* element = builder.getFloatType();
                IRIntegerValue length = 12;
                if (testCase == Case::Float3Array)
                {
                    element = builder.getVectorType(builder.getFloatType(), 3);
                    length = 4;
                }
                else if (testCase == Case::NestedMixed32)
                {
                    // Each dense record has two signed words and two float3 values. Four
                    // records exercise nested arrays and exactly the 32-register limit.
                    auto record = builder.createStructType();
                    builder.createStructField(
                        record,
                        builder.createStructKey(),
                        builder.getVectorType(builder.getIntType(), 2));
                    builder.createStructField(
                        record,
                        builder.createStructKey(),
                        builder.getArrayTypeBase(
                            kIROp_ArrayType,
                            builder.getVectorType(builder.getFloatType(), 3),
                            builder.getIntValue(builder.getIntType(), 2)));
                    element = record;
                    length = 4;
                }
                else if (testCase == Case::PaddedRecordArray)
                {
                    auto record = builder.createStructType();
                    builder.createStructField(
                        record,
                        builder.createStructKey(),
                        builder.getVectorType(builder.getFloatType(), 4));
                    builder.createStructField(
                        record,
                        builder.createStructKey(),
                        builder.getUIntType());
                    element = record;
                    length = 2;
                }
                else if (testCase == Case::BoolArray)
                    element = builder.getBoolType();
                else if (testCase == Case::HalfArray)
                    element = builder.getHalfType();
                else if (testCase == Case::UInt64Array)
                    element = builder.getUInt64Type();
                if (testCase == Case::ZeroArray)
                    length = 0;
                else if (testCase == Case::NegativeArray)
                    length = -1;
                else if (testCase == Case::OversizeArray)
                    length = 33;
                else if (testCase == Case::HugeArray)
                    length = 0x100000000LL;
                IRInst* arrayCount = builder.getIntValue(builder.getInt64Type(), length);
                if (testCase == Case::NonliteralArray)
                    arrayCount = builder.getPoison(builder.getIntType());
                IRInst* stride = nullptr;
                if (testCase == Case::NaturalStrideArray || testCase == Case::InflatedStrideArray)
                    stride = builder.getIntValue(
                        builder.getIntType(),
                        testCase == Case::NaturalStrideArray ? 4 : 8);
                leaf = builder.getArrayTypeBase(
                    testCase == Case::UnsizedArray ? kIROp_UnsizedArrayType : kIROp_ArrayType,
                    element,
                    arrayCount,
                    stride);
                payloadValid = testCase == Case::FloatArray12 || testCase == Case::Float3Array ||
                               testCase == Case::NestedMixed32 ||
                               testCase == Case::PaddedRecordArray || testCase == Case::BoolArray;
                count = testCase == Case::BoolArray           ? 3
                        : testCase == Case::NestedMixed32     ? 32
                        : testCase == Case::PaddedRecordArray ? 16
                                                              : 12;
                break;
            }
        default:
            break;
        }
        builder.createStructField(payload, builder.createStructKey(), leaf);
        if (testCase == Case::Padding)
            builder.createStructField(
                payload,
                builder.createStructKey(),
                builder.getVectorType(builder.getFloatType(), 4));
        const bool valid = payloadValid && testCase != Case::Compute &&
                           testCase != Case::IntersectionTrace &&
                           testCase != Case::DirectCallableTrace && testCase != Case::AnyHitTrace &&
                           testCase != Case::WrongOperand;
        SLANG_CHECK(getNVVMOptixPayloadRegisterCount(payload) == (payloadValid ? count : 0));
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(
                testCase == Case::Compute               ? Stage::Compute
                : testCase == Case::MissTrace           ? Stage::Miss
                : testCase == Case::ClosestHitTrace     ? Stage::ClosestHit
                : testCase == Case::IntersectionTrace   ? Stage::Intersection
                : testCase == Case::DirectCallableTrace ? Stage::Callable
                : testCase == Case::AnyHitTrace         ? Stage::AnyHit
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

// Pure global dependencies become ordinary local SSA values at every consuming use.
SLANG_UNIT_TEST(nvvmGlobalExpressionsLocalizeWithoutCloningEffects)
{
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto vectorType = builder.getVectorType(builder.getFloatType(), 3);
        IRInst* a[] = {
            builder.getFloatValue(builder.getFloatType(), 0.25),
            builder.getFloatValue(builder.getFloatType(), 0.25),
            builder.getFloatValue(builder.getFloatType(), 1)};
        IRInst* b[] = {
            builder.getFloatValue(builder.getFloatType(), 1),
            builder.getFloatValue(builder.getFloatType(), 2),
            builder.getFloatValue(builder.getFloatType(), 3)};
        auto vector = builder.emitAdd(
            vectorType,
            builder.emitMakeVector(vectorType, 3, a),
            builder.emitMakeVector(vectorType, 3, b));
        uint32_t scalarIndex[] = {1};
        auto scalarProjection = builder.emitSwizzle(builder.getFloatType(), vector, 1, scalarIndex);
        auto projectedNegation = builder.emitNeg(builder.getFloatType(), scalarProjection);
        uint32_t vectorIndices[] = {2, 0};
        auto vectorProjection = builder.emitSwizzle(
            builder.getVectorType(builder.getFloatType(), 2),
            vector,
            2,
            vectorIndices);
        auto sum = builder.emitAdd(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), 5),
            builder.getIntValue(builder.getUIntType(), 7));
        auto converted =
            builder.emitIntrinsicInst(builder.getFloatType(), kIROp_CastIntToFloat, 1, &sum);
        auto arrayType = builder.getArrayTypeBase(
            kIROp_ArrayType,
            builder.getFloatType(),
            builder.getIntValue(builder.getIntType(), 2));
        IRInst* arrayValues[] = {converted, builder.getFloatValue(builder.getFloatType(), 9)};
        auto array = builder.emitMakeArray(arrayType, 2, arrayValues);
        auto recordVectorType = builder.getVectorType(builder.getFloatType(), 2);
        auto recordVector = builder.emitMakeVector(recordVectorType, 2, arrayValues);
        auto recordType = builder.createStructType();
        auto vectorField =
            builder.createStructField(recordType, builder.createStructKey(), recordVectorType);
        auto arrayField =
            builder.createStructField(recordType, builder.createStructKey(), arrayType);
        IRInst* fields[] = {recordVector, array};
        auto record = builder.emitMakeStruct(recordType, 2, fields);
        auto helper = builder.createFunc();
        helper->setFullType(builder.getFuncType(0, nullptr, recordType));
        builder.setInsertInto(helper);
        auto helperBlock = builder.emitBlock();
        auto helperReturn = builder.emitReturn(record);

        builder.setInsertInto(module);
        auto outputType = builder.getPtrType(
            builder.getFloatType(),
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            builder.getDefaultBufferLayoutType());
        IRType* parameters[] = {outputType};
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.addKeepAliveDecoration(entry);
        builder.setInsertInto(entry);
        auto entryBlock = builder.emitBlock();
        auto output = builder.emitParam(outputType);
        auto directLane = builder.emitElementExtract(vector, IRIntegerValue(0));
        builder.emitStore(output, directLane);
        auto projectedStore = cast<IRStore>(builder.emitStore(output, projectedNegation));
        auto projectedLane = builder.emitElementExtract(vectorProjection, IRIntegerValue(0));
        builder.emitStore(output, projectedLane);
        auto directArray = builder.emitFieldExtract(record, arrayField->getKey());
        builder.emitStore(output, builder.emitElementExtract(directArray, IRIntegerValue(0)));
        auto call = builder.emitCallInst(recordType, helper, 0, nullptr);
        auto returnedVector = builder.emitFieldExtract(call, vectorField->getKey());
        builder.emitStore(output, builder.emitElementExtract(returnedVector, IRIntegerValue(1)));
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legalizeIRForNVVM(&context.codeGen, linked)));
        auto localNegation = projectedStore->getVal();
        SLANG_CHECK(localNegation->getOp() == kIROp_Neg);
        SLANG_CHECK(localNegation->getParent() == entryBlock);
        auto scalarSwizzle = localNegation->getOperand(0);
        auto vectorSwizzle = projectedLane->getOperand(0);
        for (auto projection : {scalarSwizzle, vectorSwizzle})
        {
            SLANG_CHECK(projection->getOp() == kIROp_Swizzle);
            SLANG_CHECK(projection->getParent() == entryBlock);
            SLANG_CHECK(projection->getOperand(0)->getOp() == kIROp_Add);
            SLANG_CHECK(projection->getOperand(0)->getParent() == entryBlock);
        }
        SLANG_CHECK(cast<IRIntLit>(scalarSwizzle->getOperand(1))->getValue() == 1);
        SLANG_CHECK(cast<IRIntLit>(vectorSwizzle->getOperand(1))->getValue() == 2);
        SLANG_CHECK(cast<IRIntLit>(vectorSwizzle->getOperand(2))->getValue() == 0);
        auto localRecord = directArray->getOperand(0);
        auto returnedRecord = helperReturn->getOperand(0);
        SLANG_CHECK(localRecord->getOp() == kIROp_MakeStruct);
        SLANG_CHECK(localRecord->getParent() == entryBlock);
        SLANG_CHECK(returnedRecord->getOp() == kIROp_MakeStruct);
        SLANG_CHECK(returnedRecord->getParent() == helperBlock);
        SLANG_CHECK(localRecord != returnedRecord);
        auto localVector = directLane->getOperand(0);
        SLANG_CHECK(localVector->getOp() == kIROp_Add);
        SLANG_CHECK(localVector->getParent() == entryBlock);
        for (auto local : {localRecord, returnedRecord})
        {
            auto localBlock = local->getParent();
            SLANG_CHECK(local->getOperand(0)->getOp() == kIROp_MakeVector);
            SLANG_CHECK(local->getOperand(0)->getParent() == localBlock);
            auto localArray = local->getOperand(1);
            SLANG_CHECK(localArray->getOp() == kIROp_MakeArray);
            SLANG_CHECK(localArray->getParent() == localBlock);
            auto localCast = localArray->getOperand(0);
            SLANG_CHECK(localCast->getOp() == kIROp_CastIntToFloat);
            SLANG_CHECK(localCast->getParent() == localBlock);
            SLANG_CHECK(local->getOperand(0)->getOperand(0) == localCast);
            SLANG_CHECK(localCast->getOperand(0)->getOp() == kIROp_Add);
            SLANG_CHECK(localCast->getOperand(0)->getParent() == localBlock);
        }
        // Existing checked SSA validation proves dependencies dominate each use.
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(SLANG_SUCCEEDED(result));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    for (bool useCall : {false, true})
        for (bool inConstructor : {false, true})
        {
            _resetDirectNVVMFakes();
            NVVMStaticTestContext context(unitTestContext);
            auto module = IRModule::create(context.env.getSessionImpl());
            IRBuilder builder(module);
            builder.setInsertInto(module);
            IRInst* excluded = nullptr;
            if (useCall)
            {
                auto helper = builder.createFunc();
                helper->setFullType(builder.getFuncType(0, nullptr, builder.getFloatType()));
                builder.setInsertInto(helper);
                builder.emitBlock();
                builder.emitReturn(builder.getFloatValue(builder.getFloatType(), 7));
                builder.setInsertInto(module);
                excluded = builder.emitCallInst(builder.getFloatType(), helper, 0, nullptr);
            }
            else
                excluded = builder.emitLoad(builder.createGlobalVar(builder.getFloatType()));
            IRInst* value = excluded;
            if (inConstructor)
            {
                auto sum = builder.emitAdd(
                    builder.getFloatType(),
                    builder.getFloatValue(builder.getFloatType(), 2),
                    builder.getFloatValue(builder.getFloatType(), 3));
                IRInst* lanes[] = {excluded, sum};
                value = builder.emitMakeVector(
                    builder.getVectorType(builder.getFloatType(), 2),
                    2,
                    lanes);
            }
            auto outputType = builder.getPtrType(
                builder.getFloatType(),
                AccessQualifier::ReadWrite,
                AddressSpace::UserPointer,
                builder.getDefaultBufferLayoutType());
            IRType* parameters[] = {outputType};
            auto entry = builder.createFunc();
            entry->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
            builder.addEntryPointDecoration(
                entry,
                Profile(Stage::Compute),
                toSlice("computeMain"),
                toSlice("test"));
            builder.addKeepAliveDecoration(entry);
            builder.setInsertInto(entry);
            auto block = builder.emitBlock();
            auto output = builder.emitParam(outputType);
            auto selected =
                inConstructor ? builder.emitElementExtract(value, IRIntegerValue(0)) : value;
            auto store = cast<IRStore>(builder.emitStore(output, selected));
            builder.emitReturn();
            LinkedIR linked = {};
            linked.module = module;
            linked.entryPoints.add(entry);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legalizeIRForNVVM(&context.codeGen, linked)));
            SLANG_CHECK(excluded->getParent() == module->getModuleInst());
            if (inConstructor)
            {
                auto localVector = selected->getOperand(0);
                SLANG_CHECK(localVector->getParent() == block);
                SLANG_CHECK(localVector->getOperand(0) == excluded);
                SLANG_CHECK(localVector->getOperand(1)->getOp() == kIROp_Add);
                SLANG_CHECK(localVector->getOperand(1)->getParent() == block);
            }
            else
                SLANG_CHECK(store->getVal() == excluded);
            NVVMOperationRequirements requirements;
            SLANG_CHECK(
                SLANG_FAILED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(toSlice("E52017")) >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
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

// Shared storage lowering must eliminate raw compact vector loads before preflight. Other
// collected vectors retain native storage, immutable access and trusted-root requirements.
SLANG_UNIT_TEST(nvvmConventionalGlobalVectorsUseCheckedStorage)
{
    enum class Case
    {
        Load,
        ArrayLoad,
        Store,
        ForgedRoot
    };
    for (auto elementOp : {kIROp_IntType, kIROp_UIntType, kIROp_FloatType, kIROp_HalfType})
    {
        for (uint32_t width : {2u, 3u, 4u})
        {
            if (elementOp == kIROp_HalfType && width == 2)
                continue;
            const bool compact = width == 3 || elementOp == kIROp_HalfType;
            for (auto testCase : {Case::Load, Case::ArrayLoad, Case::Store, Case::ForgedRoot})
            {
                // One representative vector is enough to check the shared permission/root gates.
                if (testCase == Case::ArrayLoad && !compact)
                    continue;
                if ((testCase == Case::Store || testCase == Case::ForgedRoot) &&
                    (elementOp != kIROp_UIntType || width != 4))
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
                IRType* fieldType = vector;
                IRStructField* arrayField = nullptr;
                if (testCase == Case::ArrayLoad)
                {
                    auto array = builder.getArrayType(
                        vector,
                        builder.getIntValue(builder.getIntType(), 2),
                        builder.getIntValue(
                            builder.getIntType(),
                            elementOp == kIROp_HalfType ? 8 : 12));
                    auto record = builder.createStructType();
                    arrayField =
                        builder.createStructField(record, builder.createStructKey(), array);
                    fieldType = builder.getType(kIROp_ConstantBufferType, record);
                }
                auto field =
                    builder.createStructField(globals, builder.createStructKey(), fieldType);
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
                    testCase == Case::ArrayLoad ? AccessQualifier::Read
                                                : AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    builder.getType(kIROp_ScalarBufferLayoutType));
                IRInst* address = builder.emitFieldAddress(
                    testCase == Case::ArrayLoad ? builder.getPtrType(
                                                      fieldType,
                                                      AccessQualifier::Read,
                                                      AddressSpace::Generic,
                                                      builder.getType(kIROp_ScalarBufferLayoutType))
                                                : pointer,
                    global,
                    field->getKey());
                if (testCase == Case::ArrayLoad)
                {
                    auto group = builder.emitLoad(fieldType, address);
                    auto arrayAddress = builder.emitFieldAddress(
                        builder.getPtrType(
                            arrayField->getFieldType(),
                            AccessQualifier::Read,
                            AddressSpace::Generic,
                            builder.getType(kIROp_ScalarBufferLayoutType)),
                        group,
                        arrayField->getKey());
                    address = builder.emitElementAddress(
                        pointer,
                        arrayAddress,
                        builder.getIntValue(builder.getIntType(), 0));
                }
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
                                      : testCase == Case::ForgedRoot
                                          ? toSlice("global_param")
                                          : toSlice("unlowered compact group load");
                const bool accepted = testCase == Case::Load && !compact;
                if ((accepted && SLANG_FAILED(result)) ||
                    (!accepted && diagnostic.indexOf(expected) < 0))
                {
                    getTestReporter()->message(
                        TestMessageType::Info,
                        context.sink.outputBuffer.getBuffer());
                }
                if (accepted)
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
                    SLANG_CHECK(load.alignment == width * 4);
                    SLANG_CHECK(load.conversion.kind == NVVMStorageConversionKind::Identity);
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

// One canonical entry contains the complete numeric family and all qualified texture geometries.
// Planning must retain reflected packing before creating a provider module.
SLANG_UNIT_TEST(nvvmResourceEntriesRetainCheckedPackingAndQueries)
{
    _resetDirectNVVMFakes();
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module);
    List<IRType*> types;
    const IROp numeric[] = {
        kIROp_BoolType,
        kIROp_Int8Type,
        kIROp_UInt8Type,
        kIROp_Int16Type,
        kIROp_UInt16Type,
        kIROp_IntType,
        kIROp_UIntType,
        kIROp_Int64Type,
        kIROp_UInt64Type,
        kIROp_HalfType,
        kIROp_FloatType,
        kIROp_DoubleType,
    };
    for (auto op : numeric)
        for (uint32_t lanes = 1; lanes <= 4; ++lanes)
        {
            auto scalar = builder.getType(op);
            types.add(lanes == 1 ? scalar : builder.getVectorType(scalar, lanes));
        }
    const IROp shapes[] = {
        kIROp_TextureShape1DType,
        kIROp_TextureShape2DType,
        kIROp_TextureShape3DType,
        kIROp_TextureShapeCubeType};
    auto zero = builder.getIntValue(builder.getIntType(), 0);
    auto one = builder.getIntValue(builder.getIntType(), 1);
    // Seven readonly and seven combined geometries, plus five writable geometries.
    for (int flavor = 0; flavor < 3; ++flavor)
        for (auto shape : shapes)
            for (bool array : {false, true})
            {
                if ((shape == kIROp_TextureShape3DType && array) ||
                    (flavor == 2 && shape == kIROp_TextureShapeCubeType))
                    continue;
                types.add(builder.getTextureType(
                    builder.getFloatType(),
                    builder.getType(shape),
                    array ? one : zero,
                    zero,
                    zero,
                    flavor == 2 ? one : zero,
                    zero,
                    flavor == 1 ? one : zero,
                    zero));
            }
    types.add(builder.getType(kIROp_SamplerStateType));
    // These layouts/flavors still lack an external entry contract.
    IRType* excluded[] = {
        builder.getType(kIROp_SamplerComparisonStateType),
        builder.getArrayType(types[48], one),
        builder.getTextureType(
            builder.getFloatType(),
            builder.getType(kIROp_TextureShape2DType),
            zero,
            one,
            one,
            zero,
            zero,
            zero,
            zero),
        builder.getTextureType(
            builder.getFloatType(),
            builder.getType(kIROp_TextureShape2DType),
            zero,
            zero,
            zero,
            zero,
            one,
            zero,
            zero),
        builder.getTextureType(
            builder.getFloatType(),
            builder.getType(kIROp_TextureShapeCubeType),
            zero,
            zero,
            zero,
            one,
            zero,
            zero,
            zero),
        builder.getTextureType(
            builder.getFloatType(),
            builder.getType(kIROp_TextureShape3DType),
            one,
            zero,
            zero,
            one,
            zero,
            zero,
            zero),
    };
    for (auto type : excluded)
    {
        SLANG_CHECK(!isNVVMSupportedParameterType(type));
        SLANG_CHECK(!classifyNVVMType(type).supports(NVVMTypeUse::EntryPointParameter));
    }
    auto entry = builder.createFunc();
    entry->setFullType(
        builder.getFuncType(types.getCount(), types.getBuffer(), builder.getVoidType()));
    builder.addEntryPointDecoration(
        entry,
        Profile(Stage::Compute),
        toSlice("computeMain"),
        toSlice("test"));
    builder.setInsertInto(entry);
    builder.emitBlock();
    List<IRParam*> params;
    for (auto type : types)
        params.add(builder.emitParam(type));
    // Query every spatial dimension and array size using actual surface handles.
    Index expectedQueries = 0;
    for (Index i = 48; i < params.getCount(); ++i)
    {
        NVVMSurfaceType surface;
        if (!getNVVMSupportedSurfaceType(params[i]->getDataType(), surface))
            continue;
        const auto rank = surface.coordinateLaneCount - (surface.isArray ? 1u : 0u);
        IRInst* operand = params[i];
        builder.emitIntrinsicInst(
            rank == 1 ? builder.getUIntType()
                      : static_cast<IRType*>(builder.getVectorType(builder.getUIntType(), rank)),
            kIROp_TextureQuerySize,
            1,
            &operand);
        ++expectedQueries;
        if (surface.isArray)
        {
            builder.emitIntrinsicInst(
                builder.getUIntType(),
                kIROp_TextureQueryLayerCount,
                1,
                &operand);
            ++expectedQueries;
        }
    }
    // Combined and writable descriptor casts keep the same opaque handle. They do not
    // manufacture a sampler object or permit a mismatched resource type.
    for (Index i = 48; i + 1 < params.getCount(); ++i)
    {
        auto texture = cast<IRTextureTypeBase>(params[i]->getDataType());
        if (!texture->isCombined() && texture->getAccess() != SLANG_RESOURCE_ACCESS_READ_WRITE)
            continue;
        auto handleType = builder.getType(kIROp_DescriptorHandleType, texture);
        IRInst* value = params[i];
        auto handle =
            builder.emitIntrinsicInst(handleType, kIROp_CastResourceToDescriptorHandle, 1, &value);
        IRInst* handleValue = handle;
        auto bits = builder.emitIntrinsicInst(
            builder.getUInt64Type(),
            kIROp_CastDescriptorHandleToUInt64,
            1,
            &handleValue);
        IRInst* bitsValue = bits;
        handleValue = builder.emitIntrinsicInst(
            handleType,
            kIROp_CastUInt64ToDescriptorHandle,
            1,
            &bitsValue);
        value = builder.emitIntrinsicInst(
            texture,
            kIROp_CastDescriptorHandleToResource,
            1,
            &handleValue);
        if (texture->isCombined())
            builder.emitIntrinsicInst(
                builder.getType(kIROp_SamplerStateType),
                kIROp_CombinedTextureSamplerGetSampler,
                1,
                &value);
    }
    builder.emitReturn();
    LinkedIR linked = {};
    linked.module = module;
    linked.entryPoints.add(entry);
    NVVMOperationRequirements requirements;
    const auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, context.sink.outputBuffer.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK(requirements.emissionPlan.entryValueParameters.getCount() == 48);
    for (Index i = 0; i < 48; ++i)
    {
        auto selected = requirements.emissionPlan.entryValueParameters.tryGetValue(params[i]);
        SLANG_CHECK_ABORT(selected);
        SLANG_CHECK(selected->type == types[i]);
        SLANG_CHECK(selected->laneCount == uint32_t(i % 4 + 1));
        SLANG_CHECK(selected->size == selected->storageLaneCount * selected->scalarBitWidth / 8);
    }
    SLANG_CHECK(requirements.textureOperations.getCount() == expectedQueries);
    for (const auto& query : requirements.textureOperations)
    {
        for (uint32_t i = 0; i < query.operationCount; ++i)
            SLANG_CHECK(
                query.operations[i].operation >= SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_WIDTH &&
                query.operations[i].operation <= SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_ARRAY_SIZE);
        if (query.source->getOp() == kIROp_TextureQueryLayerCount)
            SLANG_CHECK(
                query.operations[0].operation == SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_ARRAY_SIZE);
    }
    SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}


// A surface format belongs to its declared entry/field or explicit descriptor conversion.
// Passing the same type through an arbitrary helper does not transfer an entry annotation.
SLANG_UNIT_TEST(nvvmResourceEntryFormatsRequireDeclaredOwners)
{
    enum class Case
    {
        Native,
        HalfFormat,
        Normalized,
        WrongKind,
        HelperParameter,
        HelperResult,
        Descriptor,
        WrongDescriptor
    };
    for (auto testCase :
         {Case::Native,
          Case::HalfFormat,
          Case::Normalized,
          Case::WrongKind,
          Case::HelperParameter,
          Case::HelperResult,
          Case::Descriptor,
          Case::WrongDescriptor})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto zero = builder.getIntValue(builder.getIntType(), 0);
        auto one = builder.getIntValue(builder.getIntType(), 1);
        auto element = builder.getVectorType(builder.getFloatType(), 4);
        auto texture = builder.getTextureType(
            element,
            builder.getType(kIROp_TextureShape2DType),
            zero,
            zero,
            zero,
            one,
            zero,
            zero,
            zero);
        const bool descriptor = testCase == Case::Descriptor || testCase == Case::WrongDescriptor;
        const bool helperParam = testCase == Case::HelperParameter;
        const bool helperResult = testCase == Case::HelperResult;
        IRFunc* helper = nullptr;
        if (helperParam || helperResult)
        {
            helper = builder.createFunc();
            IRType* parameter = texture;
            helper->setFullType(builder.getFuncType(
                1,
                &parameter,
                helperResult ? static_cast<IRType*>(texture) : builder.getVoidType()));
            builder.setInsertInto(helper);
            builder.emitBlock();
            auto value = builder.emitParam(texture);
            if (helperResult)
                builder.emitReturn(value);
            else
            {
                auto coordinate = builder.emitMakeVectorFromScalar(
                    builder.getVectorType(builder.getIntType(), 2),
                    zero);
                IRInst* operands[] = {value, coordinate};
                builder.emitIntrinsicInst(element, kIROp_ImageLoad, 2, operands);
                builder.emitReturn();
            }
            builder.setInsertInto(module);
        }
        auto entry = builder.createFunc();
        IRType* parameter = descriptor ? builder.getUInt64Type() : static_cast<IRType*>(texture);
        entry->setFullType(builder.getFuncType(1, &parameter, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("computeMain"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* resource = builder.emitParam(parameter);
        if (!descriptor && testCase != Case::Native)
            builder.addFormatDecoration(
                resource,
                testCase == Case::Normalized  ? ImageFormat::rgba8
                : testCase == Case::WrongKind ? ImageFormat::rgba16ui
                                              : ImageFormat::rgba16f);
        if (descriptor)
        {
            auto handle = builder.getType(kIROp_DescriptorHandleType, texture);
            auto bits =
                builder.emitIntrinsicInst(handle, kIROp_CastUInt64ToDescriptorHandle, 1, &resource);
            IRType* resultType = texture;
            if (testCase == Case::WrongDescriptor)
                resultType = builder.getTextureType(
                    element,
                    builder.getType(kIROp_TextureShape1DType),
                    zero,
                    zero,
                    zero,
                    one,
                    zero,
                    zero,
                    zero);
            IRInst* operand = bits;
            resource = builder.emitIntrinsicInst(
                resultType,
                kIROp_CastDescriptorHandleToResource,
                1,
                &operand);
        }
        if (helperParam || helperResult)
            resource = builder.emitCallInst(helper->getResultType(), helper, 1, &resource);
        if (!helperParam)
        {
            auto coordinate = builder.emitMakeVectorFromScalar(
                builder.getVectorType(builder.getIntType(), 2),
                zero);
            IRInst* operands[] = {resource, coordinate};
            builder.emitIntrinsicInst(element, kIROp_ImageLoad, 2, operands);
        }
        builder.emitReturn();
        legalizeNVVMSurfaceOperations(module);
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = testCase == Case::Native || testCase == Case::HalfFormat ||
                           testCase == Case::Descriptor;
        if (valid && SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(SLANG_SUCCEEDED(result) == valid);
        if (valid)
        {
            SLANG_CHECK_ABORT(requirements.surfaceOperations.getCount() == 1);
            SLANG_CHECK(
                requirements.surfaceOperations[0].desc.elementType.bitWidth ==
                (testCase == Case::HalfFormat ? 16u : 32u));
        }
        else
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(toSlice("E52017")) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Layout annotations affect buffers; CUDA payload values always contain logical rows.
SLANG_UNIT_TEST(nvvmOptixPayloadAndAttributeLayoutsStayDistinct)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module);
    for (auto layout : {SLANG_MATRIX_LAYOUT_ROW_MAJOR, SLANG_MATRIX_LAYOUT_COLUMN_MAJOR})
    {
        auto matrix = builder.getMatrixType(
            builder.getFloatType(),
            builder.getIntValue(builder.getIntType(), 2),
            builder.getIntValue(builder.getIntType(), 3),
            builder.getIntValue(builder.getIntType(), layout));
        auto record = builder.createStructType();
        builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
        builder.createStructField(record, builder.createStructKey(), matrix);
        SLANG_CHECK(getNVVMOptixPayloadRegisterCount(record) == 7);
        UInt attributes = 0;
        SLANG_CHECK(getNVVMOptixAttributeRegisterCount(record, attributes) && attributes == 7);
    }
    auto record = builder.createStructType();
    builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
    builder.createStructField(
        record,
        builder.createStructKey(),
        builder.getVectorType(builder.getFloatType(), 4));
    UInt attributes = 0;
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(record) == 8);
    SLANG_CHECK(getNVVMOptixAttributeRegisterCount(record, attributes) && attributes == 5);
    auto array = builder.getArrayType(record, builder.getIntValue(builder.getIntType(), 4));
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(array) == 32);
    SLANG_CHECK(!getNVVMOptixAttributeRegisterCount(array, attributes));
    auto tooBig = builder.getArrayType(record, builder.getIntValue(builder.getIntType(), 5));
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(tooBig) == 0);

    // Empty-type legalization preserves public empty fields as Void. Their positions must
    // not consume payload words, including inside nested arrays at the 32-word boundary.
    auto chunk = builder.createStructType();
    builder.createStructField(chunk, builder.createStructKey(), builder.getVoidType());
    builder.createStructField(chunk, builder.createStructKey(), builder.getUIntType());
    builder.createStructField(chunk, builder.createStructKey(), builder.getVoidType());
    builder.createStructField(chunk, builder.createStructKey(), builder.getFloatType());
    builder.createStructField(chunk, builder.createStructKey(), builder.getVoidType());
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(chunk) == 2);
    SLANG_CHECK(!getNVVMOptixAttributeRegisterCount(chunk, attributes));
    auto nested = builder.createStructType();
    builder.createStructField(
        nested,
        builder.createStructKey(),
        builder.getArrayType(chunk, builder.getIntValue(builder.getIntType(), 16)));
    builder.createStructField(nested, builder.createStructKey(), builder.getVoidType());
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(nested) == 32);
    // Construct a new outer type rather than mutating a type with cached layout decorations.
    auto beyondLimit = builder.createStructType();
    builder.createStructField(beyondLimit, builder.createStructKey(), nested);
    builder.createStructField(beyondLimit, builder.createStructKey(), builder.getUIntType());
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(beyondLimit) == 0);

    // Ignoring a placeholder must not admit an empty root or hide an unsupported live field.
    auto empty = builder.createStructType();
    builder.createStructField(empty, builder.createStructKey(), builder.getVoidType());
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(empty) == 0);
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(builder.getVoidType()) == 0);
    builder.createStructField(empty, builder.createStructKey(), builder.getHalfType());
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(empty) == 0);

    // Boolean leaves occupy bytes and may share registers. Dense hit attributes still require
    // 32-bit leaves, while trace/callback payload counts round only the complete byte extent.
    auto boolean = builder.getBoolType();
    SLANG_CHECK(getNVVMOptixPayloadRegisterCount(boolean) == 1);
    SLANG_CHECK(!getNVVMOptixAttributeRegisterCount(boolean, attributes));
    for (int lanes = 2; lanes <= 4; ++lanes)
        SLANG_CHECK(getNVVMOptixPayloadRegisterCount(builder.getVectorType(boolean, lanes)) == 1);
    for (int length : {1, 3, 4, 5, 13, 127, 128, 129})
    {
        auto values =
            builder.getArrayType(boolean, builder.getIntValue(builder.getIntType(), length));
        SLANG_CHECK(
            getNVVMOptixPayloadRegisterCount(values) ==
            (length <= 128 ? UInt((length + 3) / 4) : 0));
        SLANG_CHECK(!getNVVMOptixAttributeRegisterCount(values, attributes));
    }
    for (int length : {1, 128, 129})
    {
        auto flags = builder.createStructType();
        builder.createStructField(flags, builder.createStructKey(), builder.getVoidType());
        for (int i = 0; i < length; ++i)
            builder.createStructField(flags, builder.createStructKey(), boolean);
        SLANG_CHECK(
            getNVVMOptixPayloadRegisterCount(flags) ==
            (length <= 128 ? UInt((length + 3) / 4) : 0));
        SLANG_CHECK(!getNVVMOptixAttributeRegisterCount(flags, attributes));
    }
    // Explicit overlapping offsets must not hide live bytes when counting physical words.
    IRType* overlappingScalars[] = {builder.getUIntType(), builder.getBoolType()};
    for (auto scalar : overlappingScalars)
    {
        auto overlapping = builder.createStructType();
        for (int i = 0; i < 2; ++i)
        {
            auto key = builder.createStructKey();
            IRInst* offset = builder.getIntValue(builder.getIntType(), 0);
            builder.addDecoration(key, kIROp_VkStructOffsetDecoration, offset);
            builder.createStructField(overlapping, key, scalar);
        }
        SLANG_CHECK(getNVVMOptixPayloadRegisterCount(overlapping) == 0);
    }
}

SLANG_UNIT_TEST(nvvmTypedBufferBindingsStayStorageOnly)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module);
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    ScopedNVVMBuilderModule scope;
    scope.builder = &provider;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(provider.createModule(toSlice("typed-binding"), scope.module)));
    NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
    SlangNVVMTypeHandle handleType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 64, handleType)));
    auto zero = builder.getIntValue(builder.getIntType(), 0);
    auto one = builder.getIntValue(builder.getIntType(), 1);
    for (bool writable : {false, true})
        for (auto scalarOp : {kIROp_FloatType, kIROp_IntType, kIROp_UIntType})
            for (int lanes = 1; lanes <= 4; ++lanes)
            {
                auto scalar = builder.getType(scalarOp);
                IRType* element = lanes == 1 ? scalar : builder.getVectorType(scalar, lanes);
                auto buffer = builder.getTextureType(
                    element,
                    builder.getType(kIROp_TextureShapeBufferType),
                    zero,
                    zero,
                    zero,
                    writable ? one : zero,
                    zero,
                    zero,
                    zero);
                auto info = classifyNVVMType(buffer);
                SLANG_CHECK(info.supports(NVVMTypeUse::Storage));
                SlangNVVMTypeHandle storage = nullptr;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(lowering.lowerType(buffer, NVVMTypeUse::Storage, storage)));
                SLANG_CHECK(storage == handleType);

                for (auto use :
                     {NVVMTypeUse::Value,
                      NVVMTypeUse::HelperValue,
                      NVVMTypeUse::HelperParameter,
                      NVVMTypeUse::HelperResult,
                      NVVMTypeUse::EntryPointParameter,
                      NVVMTypeUse::ParameterGroupStorage,
                      NVVMTypeUse::StructuredBufferStorage})
                    SLANG_CHECK(!info.supports(use));
            }
}

SLANG_UNIT_TEST(nvvmCurrentHitQueriesKeepStageAndShapeBoundaries)
{
    for (auto stage :
         {Stage::AnyHit,
          Stage::ClosestHit,
          Stage::Intersection,
          Stage::RayGeneration,
          Stage::Miss,
          Stage::Compute})
        for (uint32_t query : {16u, 17u, 18u, 21u, 22u})
            for (int invalid = 0; invalid < 4; ++invalid)
            {
                NVVMStaticTestContext context(unitTestContext);
                auto module = IRModule::create(context.env.getSessionImpl());
                IRBuilder builder(module);
                builder.setInsertInto(module);
                auto entry = builder.createFunc();
                entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
                builder.addEntryPointDecoration(
                    entry,
                    Profile(stage),
                    toSlice("probe"),
                    toSlice("test"));
                builder.setInsertInto(entry);
                builder.emitBlock();
                IRType* result =
                    query == 17 || query == 18
                        ? static_cast<IRType*>(builder.getVectorType(builder.getFloatType(), 4))
                        : builder.getUIntType();
                if (invalid == 1)
                    result = builder.getBoolType();
                IRInst* args[] = {
                    builder.getIntValue(builder.getUIntType(), query),
                    invalid == 2
                        ? builder.getPoison(builder.getUIntType())
                        : builder.getIntValue(builder.getUIntType(), invalid == 3 ? 2 : 0)};
                builder.emitIntrinsicInst(result, kIROp_OptixCurrentHitQuery, 2, args);
                builder.emitReturn();
                LinkedIR linked = {};
                linked.module = module;
                linked.entryPoints.add(entry);
                NVVMOperationRequirements requirements;
                bool valid = invalid == 0 && (stage == Stage::AnyHit || stage == Stage::ClosestHit);
                auto status = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
                SLANG_CHECK(valid == SLANG_SUCCEEDED(status));
            }
}

SLANG_UNIT_TEST(nvvmCallablePlansKeepStageAndPayloadBoundaries)
{
    for (auto stage :
         {Stage::RayGeneration,
          Stage::ClosestHit,
          Stage::Miss,
          Stage::Callable,
          Stage::AnyHit,
          Stage::Intersection,
          Stage::Compute})
        for (int shape = 0; shape < 6; ++shape)
        {
            NVVMStaticTestContext context(unitTestContext);
            auto module = IRModule::create(context.env.getSessionImpl());
            IRBuilder builder(module);
            builder.setInsertInto(module);
            auto entry = builder.createFunc();
            entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
            builder
                .addEntryPointDecoration(entry, Profile(stage), toSlice("probe"), toSlice("test"));
            builder.setInsertInto(entry);
            builder.emitBlock();
            List<IRInst*> args;
            args.add(builder.getIntValue(
                shape == 2 ? static_cast<IRType*>(builder.getIntType()) : builder.getUIntType(),
                0));
            IRType* result =
                shape == 0 ? static_cast<IRType*>(builder.getVoidType()) : builder.getUIntType();
            if (shape != 0)
                args.add(builder.getIntValue(
                    shape == 3 ? static_cast<IRType*>(builder.getIntType()) : builder.getUIntType(),
                    7));
            if (shape == 4)
                args.add(args[0]);
            if (shape == 5)
            {
                auto pointer = builder.emitVar(builder.getUIntType());
                result = pointer->getDataType();
                args[1] = pointer;
            }
            builder.emitIntrinsicInst(
                result,
                kIROp_OptixCallShader,
                args.getCount(),
                args.getBuffer());
            builder.emitReturn();
            LinkedIR linked = {};
            linked.module = module;
            linked.entryPoints.add(entry);
            NVVMOperationRequirements requirements;
            const bool valid =
                shape < 2 && (stage == Stage::RayGeneration || stage == Stage::ClosestHit ||
                              stage == Stage::Miss || stage == Stage::Callable);
            SLANG_CHECK(
                valid ==
                SLANG_SUCCEEDED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
            if (valid)
            {
                SLANG_CHECK(requirements.emissionPlan.callables.getCount() == 1);
                SLANG_CHECK(bool(requirements.emissionPlan.callables[0].payload) == (shape == 1));
            }
        }
    // Front-end out and inout both produce valid mutable payload formals. Plain values,
    // const references and multiple payloads must never be mistaken for compute parameters.
    for (int shape = 0; shape < 6; ++shape)
    {
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        IRType* type = builder.getUIntType();
        if (shape == 0 || shape == 4)
            type = builder.getBorrowInOutParamType(type);
        else if (shape == 1)
            type = builder.getOutParamType(type);
        else if (shape == 2)
            type = builder.getBorrowInParamType(type, AddressSpace::Generic);
        else if (shape == 5)
            type = builder.getBorrowInOutParamType(builder.getPtrType(type));
        IRType* params[] = {type, type};
        UInt count = shape == 4 ? 2 : 1;
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(count, params, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Callable),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        for (UInt i = 0; i < count; ++i)
            builder.emitParam(type);
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        SLANG_CHECK(
            (shape < 2) ==
            SLANG_SUCCEEDED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
    }
}

// Lowered value carriers may retain source binding layouts; storage arrays still prove strides.
SLANG_UNIT_TEST(nvvmConventionalGlobalLayoutProofsKeepRepresentationRoles)
{
    for (int testCase = 0; testCase < 3; ++testCase)
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        IRTypeLayout::Builder scalarLayoutBuilder(&builder);
        scalarLayoutBuilder.addResourceUsage(LayoutResourceKind::Uniform, 4);
        scalarLayoutBuilder.addAlignment(LayoutResourceKind::Uniform, 4);
        auto scalarLayout = scalarLayoutBuilder.build();
        IRType* fieldType = nullptr;
        IRTypeLayout* fieldLayout = nullptr;
        if (testCase == 0)
        {
            // An interface binding can specialize to a numeric record while its key retains
            // the source binding layout instead of a physical record's field topology.
            auto carrier = builder.createStructType();
            builder.createStructField(carrier, builder.createStructKey(), builder.getUIntType());
            fieldType = carrier;
            fieldLayout = scalarLayout;
        }
        else
        {
            fieldType = builder.getArrayType(
                builder.getUIntType(),
                builder.getIntValue(builder.getIntType(), 2));
            IRArrayTypeLayout::Builder arrayLayoutBuilder(&builder, scalarLayout);
            arrayLayoutBuilder.addResourceUsage(
                LayoutResourceKind::Uniform,
                testCase == 1 ? 8 : 16);
            arrayLayoutBuilder.addAlignment(LayoutResourceKind::Uniform, testCase == 1 ? 4 : 8);
            fieldLayout = arrayLayoutBuilder.build();
        }
        auto globals = builder.createStructType();
        builder.addSynthesizedParameterGroupDecoration(globals);
        auto key = builder.createStructKey();
        builder.createStructField(globals, key, fieldType);
        IRVarLayout::Builder variableLayoutBuilder(&builder, fieldLayout);
        builder.addLayoutDecoration(key, variableLayoutBuilder.build());
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
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        SLANG_CHECK(SLANG_SUCCEEDED(result) == (testCase != 2));
        if (testCase == 2)
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(
                    toSlice("aggregate storage layout")) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// The checked resource root keeps readonly permissions through fields, elements and helper calls.
SLANG_UNIT_TEST(nvvmResourceReferencesPreserveDerivedReadOnlyAccess)
{
    enum class Case
    {
        Read,
        StoreRoot,
        StoreChild,
        MutableCall,
        ReadOnlyCall,
        PointerRoot,
        PointerChild
    };
    for (bool pointerLeaf : {false, true})
        for (auto testCase :
             {Case::Read,
              Case::StoreRoot,
              Case::StoreChild,
              Case::MutableCall,
              Case::ReadOnlyCall,
              Case::PointerRoot,
              Case::PointerChild})
        {
            _resetDirectNVVMFakes();
            NVVMStaticTestContext context(unitTestContext);
            auto module = IRModule::create(context.env.getSessionImpl());
            IRBuilder builder(module);
            builder.setInsertInto(module);
            IRType* buffer = pointerLeaf ? builder.getPtrType(
                                               kIROp_PtrType,
                                               builder.getFloatType(),
                                               AccessQualifier::ReadWrite,
                                               AddressSpace::UserPointer,
                                               builder.getDefaultBufferLayoutType())
                                         : builder.getType(kIROp_HLSLByteAddressBufferType);
            auto array = builder.getArrayType(buffer, builder.getIntValue(builder.getIntType(), 2));
            auto record = builder.createStructType();
            auto field = builder.createStructField(record, builder.createStructKey(), array);
            auto readonlyRecord = builder.getBorrowInParamType(record, AddressSpace::Generic);
            auto leafReference = testCase == Case::MutableCall
                                     ? builder.getPtrType(kIROp_OutParamType, buffer)
                                     : builder.getBorrowInParamType(buffer, AddressSpace::Generic);
            auto callee = builder.createFunc();
            IRType* leafParameters[] = {leafReference};
            callee->setFullType(builder.getFuncType(1, leafParameters, builder.getVoidType()));
            builder.setInsertInto(callee);
            builder.emitBlock();
            builder.emitParam(leafReference);
            builder.emitReturn();
            builder.setInsertInto(module);
            auto helper = builder.createFunc();
            IRType* parameters[] = {readonlyRecord};
            helper->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
            builder.setInsertInto(helper);
            builder.emitBlock();
            auto value = builder.emitParam(readonlyRecord);
            auto arrayPointer = builder.getPtrType(
                array,
                AccessQualifier::Read,
                AddressSpace::Generic,
                builder.getType(kIROp_ScalarBufferLayoutType));
            auto bufferPointer = builder.getPtrType(
                buffer,
                AccessQualifier::Read,
                AddressSpace::Generic,
                builder.getType(kIROp_ScalarBufferLayoutType));
            auto fieldAddress = builder.emitFieldAddress(arrayPointer, value, field->getKey());
            auto element = builder.emitElementAddress(
                bufferPointer,
                fieldAddress,
                builder.getIntValue(builder.getIntType(), 1));
            auto loaded = builder.emitLoad(buffer, element);
            if (testCase == Case::StoreRoot)
                builder.emitStore(value, builder.emitLoad(record, value));
            if (testCase == Case::StoreChild)
                builder.emitStore(element, loaded);
            if (testCase == Case::MutableCall || testCase == Case::ReadOnlyCall)
            {
                IRInst* arguments[] = {element};
                builder.emitCallInst(builder.getVoidType(), callee, 1, arguments);
            }
            if (testCase == Case::PointerRoot || testCase == Case::PointerChild)
            {
                IRInst* source = testCase == Case::PointerRoot ? value : element;
                auto pointee = cast<IRPtrTypeBase>(source->getDataType())->getValueType();
                auto pointer = builder.getPtrType(
                    kIROp_PtrType,
                    pointee,
                    AccessQualifier::ReadWrite,
                    AddressSpace::UserPointer,
                    builder.getDefaultBufferLayoutType());
                builder.emitIntrinsicInst(pointer, kIROp_PtrCast, 1, &source);
            }
            builder.emitReturn();
            builder.setInsertInto(module);
            auto entry = builder.createFunc();
            entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
            builder.addEntryPointDecoration(
                entry,
                Profile(Stage::Compute),
                toSlice("computeMain"),
                toSlice("test"));
            builder.setInsertInto(entry);
            builder.emitBlock();
            auto local = builder.emitVar(record);
            IRInst* arguments[] = {local};
            builder.emitCallInst(builder.getVoidType(), helper, 1, arguments);
            builder.emitReturn();
            // Only reachable functions belong to executable IR; the unused callee is unnecessary.
            if (testCase != Case::MutableCall && testCase != Case::ReadOnlyCall)
                callee->removeAndDeallocate();
            LinkedIR linked = {};
            linked.module = module;
            linked.entryPoints.add(entry);
            NVVMOperationRequirements requirements;
            auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
            const bool shouldPass = testCase == Case::Read || testCase == Case::ReadOnlyCall;
            if (SLANG_SUCCEEDED(result) != shouldPass)
                getTestReporter()->message(
                    TestMessageType::Info,
                    context.sink.outputBuffer.getBuffer());
            SLANG_CHECK(SLANG_SUCCEEDED(result) == shouldPass);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
}

SLANG_UNIT_TEST(irAddressPropagationPreservesDerivedPointerProperties)
{
    NVVMStaticTestContext context(unitTestContext);
    for (bool firstEdgeAlreadyCorrect : {false, true})
    {
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder ir(module);
        ir.setInsertInto(module);
        auto array = ir.getArrayType(ir.getIntType(), ir.getIntValue(ir.getIntType(), 2));
        auto record = ir.createStructType();
        auto field = ir.createStructField(record, ir.createStructKey(), array);
        auto rootType = ir.getPtrType(
            kIROp_BorrowInParamType,
            record,
            AccessQualifier::Read,
            AddressSpace::ThreadLocal,
            ir.getDefaultBufferLayoutType());
        auto function = ir.createFunc();
        IRType* params[] = {rootType};
        function->setFullType(ir.getFuncType(1, params, ir.getVoidType()));
        ir.setInsertInto(function);
        ir.emitBlock();
        auto root = ir.emitParam(rootType);
        auto arrayPointer = ir.getPtrType(
            kIROp_PtrType,
            array,
            AccessQualifier::Read,
            firstEdgeAlreadyCorrect ? AddressSpace::ThreadLocal : AddressSpace::Generic,
            ir.getType(kIROp_ScalarBufferLayoutType));
        auto child = ir.emitFieldAddress(arrayPointer, root, field->getKey());
        // Model the explicit layouts retained by storage legalization before a later root edit.
        child->setFullType(arrayPointer);
        auto elementType = ir.getPtrType(
            kIROp_PtrType,
            ir.getIntType(),
            AccessQualifier::Read,
            AddressSpace::Generic,
            ir.getType(kIROp_ScalarBufferLayoutType));
        auto element =
            ir.emitElementAddress(elementType, child, ir.getIntValue(ir.getIntType(), 1));
        element->setFullType(elementType);
        auto header = ir.createBlock();
        IRInst* args[] = {element};
        ir.emitBranch(header, 1, args);
        ir.insertBlock(header);
        auto phi = ir.emitParam(elementType);
        auto offset = ir.emitGetOffsetPtr(phi, ir.getIntValue(ir.getIntType(), 1));
        IRInst* backedge[] = {offset};
        ir.emitBranch(header, 1, backedge);
        List<IRInst*> roots;
        roots.add(root);
        propagateAddressSpaceFromInsts(_Move(roots));
        for (auto value :
             {static_cast<IRInst*>(child),
              static_cast<IRInst*>(element),
              static_cast<IRInst*>(phi),
              static_cast<IRInst*>(offset)})
        {
            auto type = as<IRPtrTypeBase>(value->getDataType());
            SLANG_CHECK_ABORT(type);
            SLANG_CHECK(type->getOp() == kIROp_PtrType);
            SLANG_CHECK(type->getAddressSpace() == AddressSpace::ThreadLocal);
            SLANG_CHECK(type->getAccessQualifier() == AccessQualifier::Read);
            SLANG_CHECK(type->getDataLayout()->getOp() == kIROp_ScalarBufferLayoutType);
        }
    }
}

SLANG_UNIT_TEST(nvvmPointerQualificationKeepsExactPointeeAndLayout)
{
    enum class Case
    {
        Local,
        Child,
        Shared,
        SharedChild,
        LocalToShared,
        SharedToLocal,
        WrongPointee,
        ExplicitLayout,
        MissingOperand
    };
    for (auto testCase :
         {Case::Local,
          Case::Child,
          Case::Shared,
          Case::SharedChild,
          Case::LocalToShared,
          Case::SharedToLocal,
          Case::WrongPointee,
          Case::ExplicitLayout,
          Case::MissingOperand})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto record = builder.createStructType();
        auto field =
            builder.createStructField(record, builder.createStructKey(), builder.getIntType());
        const bool sharedSource = testCase == Case::Shared || testCase == Case::SharedChild ||
                                  testCase == Case::SharedToLocal;
        IRInst* shared = nullptr;
        if (sharedSource)
        {
            shared = builder.createGlobalVar(record);
            shared->setFullType(
                builder.getRateQualifiedType(builder.getGroupSharedRate(), shared->getFullType()));
        }
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* source = sharedSource ? shared : builder.emitVar(record);
        IRType* pointee = record;
        if (testCase == Case::Child || testCase == Case::SharedChild)
        {
            source = builder.emitFieldAddress(source, field->getKey());
            pointee = builder.getIntType();
            if (sharedSource)
                source->setFullType(builder.getPtrType(
                    pointee,
                    AccessQualifier::ReadWrite,
                    AddressSpace::GroupShared,
                    builder.getType(kIROp_ScalarBufferLayoutType)));
        }
        if (testCase == Case::WrongPointee)
            pointee = builder.getFloatType();
        auto pointer = builder.getPtrType(
            kIROp_PtrType,
            pointee,
            AccessQualifier::ReadWrite,
            testCase == Case::Shared || testCase == Case::SharedChild ||
                    testCase == Case::LocalToShared
                ? AddressSpace::GroupShared
                : AddressSpace::UserPointer,
            testCase == Case::ExplicitLayout ? builder.getType(kIROp_Std430BufferLayoutType)
                                             : builder.getDefaultBufferLayoutType());
        auto conversion = builder.emitIntrinsicInst(
            pointer,
            kIROp_PtrCast,
            testCase == Case::MissingOperand ? 0 : 1,
            &source);
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        const bool valid = testCase == Case::Local || testCase == Case::Child ||
                           testCase == Case::Shared || testCase == Case::SharedChild;
        if (valid != SLANG_SUCCEEDED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(valid == SLANG_SUCCEEDED(result));
        if (valid)
        {
            auto selected =
                requirements.emissionPlan.pointerQualificationValues.tryGetValue(conversion);
            SLANG_CHECK(selected && *selected == source);
            auto space = requirements.emissionPlan.scopedPointerSpaces.tryGetValue(conversion);
            SLANG_CHECK(space);
            if (space)
            {
                SLANG_CHECK(
                    space->addressSpace == (sharedSource ? SLANG_NVVM_ADDRESS_SPACE_SHARED
                                                         : SLANG_NVVM_ADDRESS_SPACE_GENERIC));
                SLANG_CHECK(space->isKnownLocalStorage == !sharedSource);
            }
        }
        else
            SLANG_CHECK(
                context.sink.outputBuffer.getUnownedSlice().indexOf(
                    toSlice("pointer qualification")) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmPointerQualificationPreservesMutableParameterRoots)
{
    for (auto op : {kIROp_OutParamType, kIROp_BorrowInOutParamType, kIROp_RefParamType})
    {
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto reference = op == kIROp_RefParamType
                             ? builder.getRefParamType(builder.getIntType(), AddressSpace::Generic)
                             : builder.getPtrType(op, builder.getIntType());
        auto pointer = builder.getPtrType(
            kIROp_PtrType,
            builder.getIntType(),
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            builder.getDefaultBufferLayoutType());
        auto helper = builder.createFunc();
        IRType* types[] = {reference};
        helper->setFullType(builder.getFuncType(1, types, builder.getVoidType()));
        builder.setInsertInto(helper);
        builder.emitBlock();
        IRInst* source = builder.emitParam(reference);
        auto conversion = builder.emitIntrinsicInst(pointer, kIROp_PtrCast, 1, &source);
        builder.emitReturn();
        builder.setInsertInto(module);
        auto entry = builder.createFunc();
        entry->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        IRInst* local = builder.emitVar(builder.getIntType());
        builder.emitCallInst(builder.getVoidType(), helper, 1, &local);
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                context.sink.outputBuffer.getBuffer());
        SLANG_CHECK(SLANG_SUCCEEDED(result));
        auto selected =
            requirements.emissionPlan.pointerQualificationValues.tryGetValue(conversion);
        SLANG_CHECK(selected && *selected == source);
    }
}

// Retained declarations need no physical type until a checked executable/storage role uses them.
SLANG_UNIT_TEST(nvvmRetainedStructDeclarationsKeepLiveRoleChecks)
{
    enum class Use
    {
        Metadata,
        Local,
        Entry,
        Helper,
        Global,
        ConventionalGlobal,
        ExtraFunction
    };
    for (auto use :
         {Use::Metadata,
          Use::Local,
          Use::Entry,
          Use::Helper,
          Use::Global,
          Use::ConventionalGlobal,
          Use::ExtraFunction})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto child = builder.createStructType();
        builder.createStructField(
            child,
            builder.createStructKey(),
            builder.getType(kIROp_StringType));
        auto retained = builder.createStructType();
        builder.createStructField(retained, builder.createStructKey(), child);
        builder.addKeepAliveDecoration(retained);
        IRFunc* helper = nullptr;
        if (use == Use::Helper || use == Use::ExtraFunction)
        {
            helper = builder.createFunc();
            IRType* params[] = {child};
            helper->setFullType(
                builder.getFuncType(use == Use::Helper ? 1 : 0, params, builder.getVoidType()));
            builder.setInsertInto(helper);
            builder.emitBlock();
            if (use == Use::Helper)
                builder.emitParam(child);
            builder.emitReturn();
            builder.setInsertInto(module);
        }
        if (use == Use::Global)
            builder.createGlobalVar(child);
        if (use == Use::ConventionalGlobal)
        {
            auto globals = builder.createStructType();
            builder.addSynthesizedParameterGroupDecoration(globals);
            builder.createStructField(globals, builder.createStructKey(), child);
            builder.createGlobalParam(builder.getType(kIROp_ConstantBufferType, globals));
        }
        auto entry = builder.createFunc();
        IRType* params[] = {child};
        entry->setFullType(
            builder.getFuncType(use == Use::Entry ? 1 : 0, params, builder.getVoidType()));
        builder.addEntryPointDecoration(
            entry,
            Profile(Stage::Compute),
            toSlice("probe"),
            toSlice("test"));
        builder.setInsertInto(entry);
        builder.emitBlock();
        if (use == Use::Entry)
            builder.emitParam(child);
        if (use == Use::Local)
            builder.emitVar(child);
        if (use == Use::Helper)
        {
            IRInst* arg = builder.getPoison(child);
            builder.emitCallInst(builder.getVoidType(), helper, 1, &arg);
        }
        builder.emitReturn();
        LinkedIR linked = {};
        linked.module = module;
        linked.entryPoints.add(entry);
        NVVMOperationRequirements requirements;
        auto result = validateNVVMSupportedIR(&context.codeGen, linked, requirements);
        SLANG_CHECK((use == Use::Metadata) == SLANG_SUCCEEDED(result));
        if (use != Use::Metadata)
        {
            auto diagnostic = context.sink.outputBuffer.getUnownedSlice();
            SLANG_CHECK(diagnostic.indexOf(toSlice("E52017")) >= 0);
            const char* role = use == Use::Local                ? "'var'"
                               : use == Use::Entry              ? "entry-point parameter"
                               : use == Use::Helper             ? "helper function parameter"
                               : use == Use::Global             ? "'global_var'"
                               : use == Use::ConventionalGlobal ? "conventional global field"
                                                                : "'func'";
            if (diagnostic.indexOf(UnownedStringSlice(role)) < 0)
                getTestReporter()->message(
                    TestMessageType::Info,
                    context.sink.outputBuffer.getBuffer());
            SLANG_CHECK(diagnostic.indexOf(UnownedStringSlice(role)) >= 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}
