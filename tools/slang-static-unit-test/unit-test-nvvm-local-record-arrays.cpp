// Direct tests of local record-array roles and linked-IR address provenance.
#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm.h"

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
