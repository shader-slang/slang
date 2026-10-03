// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Direct tests keep canonical value, physical helper ABI, and storage roles distinct.
#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm-type-lowering.h"
#include "slang/slang-ir-layout.h"

using namespace Slang;

namespace
{

// Each fresh lowering context starts with a different role. The independently constructed LLVM
// handles distinguish native Half values, integer boundary transport, and compact CUDA storage.
void checkHalfBoundaryCacheOrders(UnitTestContext* testContext, uint32_t laneCount)
{
    NVVMStaticTestContext context(testContext);
    RefPtr<IRModule> irModule = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(irModule.get());
    ir.setInsertInto(irModule.get());
    IRType* half = ir.getType(kIROp_HalfType);
    IRType* canonical = laneCount == 1 ? half : ir.getVectorType(half, laneCount);
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(testContext, provider);
    const NVVMTypeUse roles[] = {
        NVVMTypeUse::HelperValue,
        NVVMTypeUse::Value,
        NVVMTypeUse::Storage,
        NVVMTypeUse::HelperParameter,
        NVVMTypeUse::HelperResult,
    };
    for (Index first = 0; first < SLANG_COUNT_OF(roles); ++first)
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.createModule(toSlice("half-boundary-cache"), scope.module)));
        SlangNVVMTypeHandle nativeHalf = nullptr;
        SlangNVVMTypeHandle integer = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.getFloatingPointType(scope.module, 16, nativeHalf)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 16, integer)));
        SlangNVVMTypeHandle value = nativeHalf;
        SlangNVVMTypeHandle boundary = integer;
        if (laneCount > 1)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                provider.getVectorType(scope.module, nativeHalf, laneCount, value)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                provider.getVectorType(scope.module, integer, laneCount, boundary)));
        }
        SlangNVVMTypeHandle storage = value;
        if (laneCount >= 3)
        {
            SlangNVVMTypeHandle chunk = nullptr;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(provider.getVectorType(scope.module, nativeHalf, 2, chunk)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(provider.getArrayType(scope.module, chunk, 2, storage)));
        }
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        // The first pass populates caches; the second proves subsequent requests stay role-correct.
        for (Index request = 0; request < 2 * SLANG_COUNT_OF(roles); ++request)
        {
            const auto role = roles[(first + request) % SLANG_COUNT_OF(roles)];
            SlangNVVMTypeHandle actual = boundary;
            const auto result = lowering.lowerType(canonical, role, actual);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            const auto expected =
                role == NVVMTypeUse::Storage ? storage
                : role == NVVMTypeUse::HelperParameter || role == NVVMTypeUse::HelperResult
                    ? boundary
                    : value;
            SLANG_CHECK(actual == expected);
        }
        for (auto forbidden : {NVVMTypeUse::EntryPointResult})
        {
            SlangNVVMTypeHandle actual = boundary;
            SLANG_CHECK(SLANG_FAILED(lowering.lowerType(canonical, forbidden, actual)));
            SLANG_CHECK(actual == nullptr);
        }
    }
}

} // namespace

// Copyable HelperValue requests normalize to Value before cache lookup. Preserve this existing
// scalar invariant while extending physical boundary transport to Half vectors.
SLANG_UNIT_TEST(nvvmScalarHalfBoundaryRolesIgnoreCacheOrder)
{
    checkHalfBoundaryCacheOrders(unitTestContext, 1);
}

// Half vector boundaries use integer lane bits while value and storage roles keep their own types.
SLANG_UNIT_TEST(nvvmHalfVectorBoundaryRolesIgnoreCacheOrder)
{
    for (uint32_t laneCount = 2; laneCount <= 4; ++laneCount)
        checkHalfBoundaryCacheOrders(unitTestContext, laneCount);
}

// Classify only the existing scalar Half and fixed two- through four-lane Half value types.
// A pointer, array, other scalar family, or unsupported width cannot acquire this boundary ABI.
SLANG_UNIT_TEST(nvvmHalfHelperABIClassifierUsesExactValueTypes)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder ir(module.get());
    ir.setInsertInto(module.get());
    IRType* half = ir.getType(kIROp_HalfType);
    IRType* floating = ir.getFloatType();
    IRType* bfloat = ir.getType(kIROp_BFloat16Type);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(half) == 1);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(floating) == 0);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(bfloat) == 0);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getUInt16Type()) == 0);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getVectorType(half, 1)) == 0);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getVectorType(half, 5)) == 0);
    SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getPtrType(kIROp_PtrType, half)) == 0);
    for (uint32_t width = 2; width <= 4; ++width)
    {
        auto vector = ir.getVectorType(half, width);
        SLANG_CHECK(getNVVMHalfHelperABILaneCount(vector) == width);
        SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getVectorType(floating, width)) == 0);
        SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getVectorType(bfloat, width)) == 0);
        SLANG_CHECK(getNVVMHalfHelperABILaneCount(ir.getPtrType(kIROp_PtrType, vector)) == 0);
        auto array =
            ir.getArrayTypeBase(kIROp_ArrayType, vector, ir.getIntValue(ir.getIntType(), 2));
        SLANG_CHECK(getNVVMHalfHelperABILaneCount(array) == 0);
    }
}

// BF3/BF4 records have qualified local storage, but their component-array memory representation
// does not grant a whole-record value ABI. Preserve that distinction before and after caching.
SLANG_UNIT_TEST(nvvmLocalBFloat16RecordStorageKeepsRolesSeparate)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    const NVVMTypeUse forbidden[] = {
        NVVMTypeUse::Value,
        NVVMTypeUse::HelperValue,
        NVVMTypeUse::HelperParameter,
        NVVMTypeUse::HelperResult,
        NVVMTypeUse::ParameterGroupStorage,
        NVVMTypeUse::StructuredBufferStorage,
        NVVMTypeUse::EntryPointParameter,
        NVVMTypeUse::EntryPointResult,
    };
    for (uint32_t width : {3u, 4u})
    {
        auto record = ir.createStructType();
        ir.createStructField(record, ir.createStructKey(), ir.getType(kIROp_UInt16Type));
        ir.createStructField(
            record,
            ir.createStructKey(),
            ir.getVectorType(ir.getType(kIROp_BFloat16Type), width));
        ir.createStructField(record, ir.createStructKey(), ir.getType(kIROp_UInt16Type));
        SLANG_CHECK(asNVVMSupportedLocalBFloat16RecordType(record) == record);
        SLANG_CHECK(asNVVMSupportedSubstandardRecordType(record) == nullptr);
        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            provider.createModule(toSlice("local-bfloat-record-storage-roles"), scope.module)));
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        SlangNVVMTypeHandle cachedStorage = nullptr;
        for (Index pass = 0; pass < 2; ++pass)
        {
            for (auto use : forbidden)
            {
                SlangNVVMTypeHandle actual = cachedStorage;
                const auto result = lowering.lowerType(record, use, actual);
                if (SLANG_SUCCEEDED(result) || actual)
                {
                    StringBuilder message;
                    message << "BF" << width << " record role " << int(use) << ", cache pass "
                            << pass;
                    getTestReporter()->message(TestMessageType::Info, message.getBuffer());
                }
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(actual == nullptr);
            }
            SlangNVVMTypeHandle storage = nullptr;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(lowering.lowerType(record, NVVMTypeUse::Storage, storage)));
            SLANG_CHECK(storage != nullptr);
            if (cachedStorage)
                SLANG_CHECK(storage == cachedStorage);
            cachedStorage = storage;
        }
    }
}

// Parameter-group storage may contain direct pointer fields without creating a value ABI for
// their enclosing record. Cache order must not authorize ordinary storage or helper values.
SLANG_UNIT_TEST(nvvmParameterGroupLayoutPointerStorageKeepsRolesSeparate)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    auto record = ir.createStructType();
    ir.createStructField(record, ir.createStructKey(), ir.getUIntType());
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    const NVVMTypeUse forbidden[] = {
        NVVMTypeUse::Value,
        NVVMTypeUse::Storage,
        NVVMTypeUse::HelperValue,
        NVVMTypeUse::HelperParameter,
        NVVMTypeUse::HelperResult,
        NVVMTypeUse::StructuredBufferStorage,
    };
    for (auto layoutOp :
         {kIROp_Std430BufferLayoutType, kIROp_ScalarBufferLayoutType, kIROp_CBufferLayoutType})
    {
        auto pointer = ir.getPtrType(
            record,
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            ir.getType(layoutOp));
        auto fields = ir.createStructType();
        ir.createStructField(fields, ir.createStructKey(), pointer);
        auto nested = ir.createStructType();
        ir.createStructField(nested, ir.createStructKey(), fields);
        auto pointerArray =
            ir.getArrayTypeBase(kIROp_ArrayType, pointer, ir.getIntValue(ir.getIntType(), 2));
        auto recordArray =
            ir.getArrayTypeBase(kIROp_ArrayType, fields, ir.getIntValue(ir.getIntType(), 2));
        SLANG_CHECK(isNVVMSupportedParameterGroupElementStorageType(fields));
        SLANG_CHECK(!hasNVVMParameterGroupStorageValueRepresentation(fields));
        for (IRType* excluded :
             {static_cast<IRType*>(nested),
              static_cast<IRType*>(pointerArray),
              static_cast<IRType*>(recordArray)})
            SLANG_CHECK(!isNVVMSupportedParameterGroupElementStorageType(excluded));

        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            provider.createModule(toSlice("parameter-group-pointer-roles"), scope.module)));
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        for (Index pass = 0; pass < 2; ++pass)
        {
            for (auto use : forbidden)
            {
                SlangNVVMTypeHandle actual = nullptr;
                const auto result = lowering.lowerType(fields, use, actual);
                if (SLANG_SUCCEEDED(result) || actual)
                {
                    StringBuilder message;
                    message << "layout " << getIROpInfo(layoutOp).name << ", role " << int(use)
                            << ", cache pass " << pass;
                    getTestReporter()->message(TestMessageType::Info, message.getBuffer());
                }
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(actual == nullptr);
            }
            SlangNVVMTypeHandle storage = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                lowering.lowerType(fields, NVVMTypeUse::ParameterGroupStorage, storage)));
            SLANG_CHECK(storage != nullptr);
            SlangNVVMTypeHandle leaf = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                lowering.lowerType(pointer, NVVMTypeUse::ParameterGroupStorage, leaf)));
            SLANG_CHECK(leaf != nullptr);
        }
    }
}

// A real OptiX handle may be loaded from launch storage and forwarded to TraceRay. Caching its
// UInt64 physical type must not grant integer semantics, returned handles, or aggregate storage.
SLANG_UNIT_TEST(nvvmAccelerationHandlesKeepOpaqueRoles)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    auto handle = ir.getType(kIROp_RaytracingAccelerationStructureType);
    auto record = ir.createStructType();
    ir.createStructField(record, ir.createStructKey(), handle);
    auto globals = ir.createStructType();
    ir.addSynthesizedParameterGroupDecoration(globals);
    auto field = ir.createStructField(globals, ir.createStructKey(), handle);
    auto array = ir.getArrayTypeBase(kIROp_ArrayType, handle, ir.getIntValue(ir.getIntType(), 2));
    SLANG_CHECK(isNVVMSupportedConventionalGlobalFieldType(field));
    SLANG_CHECK(getNVVMResourceValueAlignment(handle) == 8);
    SLANG_CHECK(getNVVMResourceValueAlignment(record) == 0);
    SLANG_CHECK(getNVVMResourceValueAlignment(array) == 0);
    SLANG_CHECK(!isNVVMSupportedIntegerScalarType(handle));
    SLANG_CHECK(!asNVVMSupportedResourceStructType(record));
    SLANG_CHECK(!asNVVMSupportedResourceArrayType(array));
    SLANG_CHECK(!isNVVMSupportedStructuredBufferStorageType(record));
    SLANG_CHECK(!asNVVMSupportedParameterGroupType(ir.getType(kIROp_ConstantBufferType, record)));

    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    const NVVMTypeUse admitted[] = {
        NVVMTypeUse::Value,
        NVVMTypeUse::Storage,
        NVVMTypeUse::HelperParameter,
    };
    const NVVMTypeUse excluded[] = {
        NVVMTypeUse::HelperValue,
        NVVMTypeUse::HelperResult,
        NVVMTypeUse::EntryPointParameter,
        NVVMTypeUse::EntryPointResult,
        NVVMTypeUse::ParameterGroupStorage,
        NVVMTypeUse::StructuredBufferStorage,
    };
    for (Index first = 0; first < SLANG_COUNT_OF(admitted); ++first)
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            provider.createModule(toSlice("opaque-acceleration-handle"), scope.module)));
        SlangNVVMTypeHandle integer = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 64, integer)));
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        for (Index i = 0; i < SLANG_COUNT_OF(admitted); ++i)
        {
            SlangNVVMTypeHandle actual = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(
                handle,
                admitted[(first + i) % SLANG_COUNT_OF(admitted)],
                actual)));
            SLANG_CHECK(actual == integer);
        }
        SlangNVVMTypeHandle actual = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(lowering.lowerType(globals, NVVMTypeUse::Storage, actual)));
        for (auto role : excluded)
        {
            actual = integer;
            SLANG_CHECK(SLANG_FAILED(lowering.lowerType(handle, role, actual)));
            SLANG_CHECK(actual == nullptr);
        }
        for (auto type :
             {static_cast<IRType*>(record),
              static_cast<IRType*>(array),
              static_cast<IRType*>(ir.getPtrType(kIROp_PtrType, handle))})
        {
            for (auto role : admitted)
            {
                actual = integer;
                SLANG_CHECK(SLANG_FAILED(lowering.lowerType(type, role, actual)));
                SLANG_CHECK(actual == nullptr);
            }
        }
    }
}

// CUDA parameter storage and ordinary SSA remain distinct even when either cache is populated
// first.
SLANG_UNIT_TEST(nvvmNumericEntryCarriersPreserveCudaPacking)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    const IROp scalarOps[] = {
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
    const uint32_t bytes[] = {1, 1, 1, 2, 2, 4, 4, 8, 8, 2, 4, 8};
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    for (Index kind = 0; kind < SLANG_COUNT_OF(scalarOps); ++kind)
        for (uint32_t lanes = 1; lanes <= 4; ++lanes)
        {
            auto scalar = ir.getType(scalarOps[kind]);
            IRType* type = lanes == 1 ? scalar : ir.getVectorType(scalar, lanes);
            const bool half = scalarOps[kind] == kIROp_HalfType;
            const bool boolean = scalarOps[kind] == kIROp_BoolType;
            const auto storedLanes = half && lanes == 3 ? 4u : lanes;
            const auto expectedSize = storedLanes * bytes[kind];
            const auto expectedAlignment = half && lanes >= 3 ? 4u
                                           : lanes == 3       ? bytes[kind]
                                                              : Math::Min(16u, lanes * bytes[kind]);
            NVVMCUDAValueLayout layout;
            SLANG_CHECK_ABORT(getNVVMCUDAValueLayout(&context.codeGen, type, layout));
            SLANG_CHECK(layout.size == expectedSize && layout.alignment == expectedAlignment);
            SLANG_CHECK(layout.laneCount == lanes && layout.storageLaneCount == storedLanes);
            SLANG_CHECK(layout.scalarBitWidth == bytes[kind] * 8);
            SLANG_CHECK(layout.isBoolean == boolean && layout.isHalf == half);
            SLANG_CHECK(isNVVMSupportedParameterType(type));
            SLANG_CHECK(classifyNVVMType(type).supports(NVVMTypeUse::EntryPointParameter));
            for (bool entryFirst : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &provider;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(provider.createModule(toSlice("entry-numeric"), scope.module)));
                NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
                SlangNVVMTypeHandle logical = nullptr;
                SlangNVVMTypeHandle actual = nullptr;
                if (!entryFirst)
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(lowering.lowerType(type, NVVMTypeUse::Value, logical)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    lowering.lowerType(type, NVVMTypeUse::EntryPointParameter, actual)));
                SlangNVVMTypeHandle physicalScalar = nullptr;
                if (half || boolean)
                {
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        provider.getIntegerType(scope.module, bytes[kind] * 8, physicalScalar)));
                }
                else
                {
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        lowering.lowerType(scalar, NVVMTypeUse::Value, physicalScalar)));
                }
                SlangNVVMTypeHandle expected = physicalScalar;
                if (lanes > 1)
                {
                    SlangNVVMTypeHandle array = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        provider.getArrayType(scope.module, physicalScalar, storedLanes, array)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
                        scope.module,
                        array,
                        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
                        expected)));
                }
                SLANG_CHECK(actual == expected);
                SlangNVVMTypeHandle after = nullptr;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(lowering.lowerType(type, NVVMTypeUse::Value, after)));
                if (!entryFirst)
                    SLANG_CHECK(after == logical);
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    lowering.lowerType(type, NVVMTypeUse::EntryPointParameter, actual)));
                SLANG_CHECK(actual == expected);
                if (lanes > 1 || half || boolean)
                    SLANG_CHECK(after != actual);
            }
        }
    for (auto type :
         {ir.getType(kIROp_BFloat16Type),
          ir.getType(kIROp_FloatE4M3Type),
          ir.getType(kIROp_FloatE5M2Type)})
    {
        NVVMCUDAValueLayout layout;
        SLANG_CHECK(!getNVVMCUDAValueLayout(&context.codeGen, type, layout));
        SLANG_CHECK(!isNVVMSupportedParameterType(type));
    }
}


SLANG_UNIT_TEST(nvvmNumericAggregateEntryLayoutsRejectMismatchedStrides)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    auto count = ir.getIntValue(ir.getIntType(), 2);
    auto float3 = ir.getVectorType(ir.getFloatType(), 3);
    auto compact = ir.getArrayType(float3, count);
    NVVMCUDAValueLayout layout;
    SLANG_CHECK_ABORT(getNVVMCUDAValueLayout(&context.codeGen, compact, layout));
    SLANG_CHECK(layout.size == 24 && layout.alignment == 4 && layout.elementStride == 12);
    SLANG_CHECK(layout.elementCount == 2 && layout.children.getCount() == 1);
    SLANG_CHECK(layout.children[0].laneCount == 3);
    auto padded = ir.getArrayType(float3, count, ir.getIntValue(ir.getIntType(), 16));
    SLANG_CHECK(isNVVMSupportedCopyableValueType(padded));
    SLANG_CHECK(!getNVVMCUDAValueLayout(&context.codeGen, padded, layout));
    auto scalar = ir.getArrayType(ir.getFloatType(), count, ir.getIntValue(ir.getIntType(), 4));
    SLANG_CHECK_ABORT(getNVVMCUDAValueLayout(&context.codeGen, scalar, layout));
    SLANG_CHECK(layout.size == 8 && layout.elementStride == 4);
    auto nested = ir.getArrayType(compact, count);
    SLANG_CHECK_ABORT(getNVVMCUDAValueLayout(&context.codeGen, nested, layout));
    SLANG_CHECK(layout.size == 48 && layout.elementStride == 24);
    SLANG_CHECK(!getNVVMCUDAValueLayout(
        &context.codeGen,
        ir.getArrayType(ir.getFloatType(), ir.getIntValue(ir.getIntType(), 0)),
        layout));
    SLANG_CHECK(!getNVVMCUDAValueLayout(
        &context.codeGen,
        ir.getArrayType(ir.getDoubleType(), ir.getIntValue(ir.getIntType(), UINT32_MAX)),
        layout));
}

SLANG_UNIT_TEST(nvvmResourceHelperRolesPreserveStorageAndAccess)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    auto buffer = ir.getType(kIROp_HLSLByteAddressBufferType);
    auto pair = ir.getArrayType(buffer, ir.getIntValue(ir.getIntType(), 2));
    auto numeric = ir.getFloatType();
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    for (auto firstRole :
         {NVVMTypeUse::HelperParameter, NVVMTypeUse::HelperResult, NVVMTypeUse::Value})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.createModule(toSlice("resource-helper-roles"), scope.module)));
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        SlangNVVMTypeHandle first = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(pair, firstRole, first)));
        for (auto role :
             {NVVMTypeUse::HelperParameter, NVVMTypeUse::HelperResult, NVVMTypeUse::Value})
        {
            SlangNVVMTypeHandle next = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(pair, role, next)));
            SLANG_CHECK(next == first);
        }
        SLANG_CHECK(!classifyNVVMType(pair).supports(NVVMTypeUse::EntryPointParameter));
        SLANG_CHECK(!classifyNVVMType(pair).supports(NVVMTypeUse::HelperValue));
    }
    for (auto value : {buffer, static_cast<IRType*>(pair)})
    {
        for (auto op : {kIROp_PtrType, kIROp_OutParamType, kIROp_BorrowInOutParamType})
        {
            auto reference = ir.getPtrType(op, value);
            SLANG_CHECK(asNVVMSupportedLocalResourceValuePointerType(reference));
            SLANG_CHECK(classifyNVVMType(reference).supports(NVVMTypeUse::HelperParameter));
            SLANG_CHECK(!classifyNVVMType(reference).supports(NVVMTypeUse::HelperResult));
        }
        auto readonly = ir.getBorrowInParamType(value, AddressSpace::Generic);
        SLANG_CHECK(asNVVMSupportedLocalResourceValuePointerType(readonly));
        SLANG_CHECK(readonly->getAccessQualifier() == AccessQualifier::Read);
        SLANG_CHECK(asNVVMSupportedLocalResourceValuePointerType(
            ir.getRefParamType(value, AddressSpace::Generic)));
        for (auto space :
             {AddressSpace::GroupShared, AddressSpace::UserPointer, AddressSpace::ThreadLocal})
            SLANG_CHECK(!asNVVMSupportedLocalResourceValuePointerType(ir.getPtrType(value, space)));
    }
    SLANG_CHECK(!asNVVMSupportedLocalResourceValuePointerType(ir.getPtrType(numeric)));
    IRSizeAndAlignment layout;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getSizeAndAlignment(
        context.codeGen.getTargetReq(),
        IRTypeLayoutRules::getCUDA(),
        buffer,
        &layout)));
    SLANG_CHECK(layout.size == 16 && layout.alignment == 8);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getSizeAndAlignment(
        context.codeGen.getTargetReq(),
        IRTypeLayoutRules::getLLVM(),
        pair,
        &layout)));
    SLANG_CHECK(layout.size == 32 && layout.alignment == 8);
}

SLANG_UNIT_TEST(nvvmRawBufferLayoutMatchesCpuCudaDescriptors)
{
    for (auto format : {SLANG_PTX, SLANG_CPP_SOURCE, SLANG_SPIRV})
    {
        NVVMStaticTestContext context(unitTestContext);
        slang::TargetDesc desc = {};
        desc.format = format;
        auto linkage = context.owner->getLinkage();
        linkage->addTarget(desc);
        auto target = linkage->targets.getLast();
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder ir(module);
        ir.setInsertInto(module);
        for (auto op : {kIROp_HLSLByteAddressBufferType, kIROp_HLSLRWByteAddressBufferType})
        {
            auto buffer = ir.getType(op);
            IRSizeAndAlignment layout;
            // Targetless IR queries cannot choose a CPU/CUDA descriptor representation.
            SLANG_CHECK(SLANG_FAILED(
                getSizeAndAlignment(nullptr, IRTypeLayoutRules::getNatural(), buffer, &layout)));
            auto result =
                getSizeAndAlignment(target, IRTypeLayoutRules::getNatural(), buffer, &layout);
            if (format == SLANG_SPIRV)
            {
                SLANG_CHECK(SLANG_FAILED(result));
            }
            else
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
                SLANG_CHECK(layout.size == 16 && layout.alignment == 8);
                auto pair = ir.getArrayType(buffer, ir.getIntValue(ir.getIntType(), 2));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    getSizeAndAlignment(target, IRTypeLayoutRules::getNatural(), pair, &layout)));
                SLANG_CHECK(layout.size == 32 && layout.alignment == 8);
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmPointerEntryLayoutsKeepLaunchAndHelperRolesSeparate)
{
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder ir(module);
    ir.setInsertInto(module);
    auto pointer = ir.getPtrType(
        kIROp_PtrType,
        ir.getIntType(),
        AccessQualifier::ReadWrite,
        AddressSpace::UserPointer,
        ir.getDefaultBufferLayoutType());
    auto record = ir.createStructType();
    ir.createStructField(record, ir.createStructKey(), ir.getUInt8Type());
    ir.createStructField(record, ir.createStructKey(), pointer);
    auto array = ir.getArrayType(record, ir.getIntValue(ir.getIntType(), 2));
    NVVMCUDAValueLayout layout;
    SLANG_CHECK_ABORT(getNVVMCUDAValueLayout(&context.codeGen, array, layout));
    SLANG_CHECK(layout.elementCount == 2 && layout.elementStride == 16);
    SLANG_CHECK(layout.children[0].fieldOffsets[1] == 8);
    auto& leaf = layout.children[0].children[1];
    SLANG_CHECK(leaf.isUserPointer && leaf.size == 8 && leaf.alignment == 8);
    auto halfRecord = ir.createStructType();
    ir.createStructField(halfRecord, ir.createStructKey(), pointer);
    ir.createStructField(halfRecord, ir.createStructKey(), ir.getHalfType());
    SLANG_CHECK(hasNVVMHalfHelperABITransport(halfRecord));
    NVVMIRBuilder provider;
    _requireRealNVVMBuilder(unitTestContext, provider);
    for (bool entryFirst : {false, true})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &provider;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.createModule(toSlice("pointer-entry"), scope.module)));
        NVVMTypeLoweringContext lowering(&context.codeGen, provider, scope.module);
        SlangNVVMTypeHandle entry = nullptr, value = nullptr, after = nullptr;
        if (!entryFirst)
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(lowering.lowerType(array, NVVMTypeUse::Value, value)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(lowering.lowerType(array, NVVMTypeUse::EntryPointParameter, entry)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(lowering.lowerType(array, NVVMTypeUse::Value, after)));
        SLANG_CHECK(entry != after);
        if (value)
            SLANG_CHECK(value == after);
        SlangNVVMTypeHandle launchPointer = nullptr, storage = nullptr, helperPointer = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(lowering.lowerCUDAValueType(leaf, launchPointer, storage)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            lowering.lowerType(pointer, NVVMTypeUse::HelperParameter, helperPointer)));
        SLANG_CHECK(launchPointer == storage && launchPointer != helperPointer);
        SlangNVVMTypeHandle integer = nullptr, expectedGlobal = nullptr, expectedGeneric = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 32, integer)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
            scope.module,
            integer,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            expectedGlobal)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
            scope.module,
            integer,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            expectedGeneric)));
        SLANG_CHECK(launchPointer == expectedGlobal && helperPointer == expectedGeneric);
        auto recordPointer = ir.getPtrType(
            kIROp_PtrType,
            record,
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            ir.getDefaultBufferLayoutType());
        auto indirectPointer = ir.getPtrType(
            kIROp_PtrType,
            recordPointer,
            AccessQualifier::ReadWrite,
            AddressSpace::UserPointer,
            ir.getDefaultBufferLayoutType());
        SlangNVVMTypeHandle indirectEntry = nullptr, indirectHelper = nullptr;
        if (!entryFirst)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                lowering.lowerType(indirectPointer, NVVMTypeUse::HelperParameter, indirectHelper)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            lowering.lowerType(indirectPointer, NVVMTypeUse::EntryPointParameter, indirectEntry)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            lowering.lowerType(indirectPointer, NVVMTypeUse::HelperParameter, indirectHelper)));
        SlangNVVMTypeHandle byteType = nullptr, expectedRecord = nullptr,
                            expectedRecordPointer = nullptr, expectedIndirectEntry = nullptr,
                            expectedIndirectHelper = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 8, byteType)));
        SlangNVVMTypeHandle recordFields[] = {byteType, expectedGeneric};
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.getStructType(scope.module, recordFields, 2, expectedRecord)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
            scope.module,
            expectedRecord,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            expectedRecordPointer)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
            scope.module,
            expectedRecordPointer,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            expectedIndirectEntry)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getPointerType(
            scope.module,
            expectedRecordPointer,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            expectedIndirectHelper)));
        SLANG_CHECK(indirectEntry == expectedIndirectEntry);
        SLANG_CHECK(indirectHelper == expectedIndirectHelper);
        SlangNVVMTypeHandle ordinaryHalfRecord = nullptr, physicalHalfRecord = nullptr;
        if (!entryFirst)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                lowering.lowerType(halfRecord, NVVMTypeUse::Value, ordinaryHalfRecord)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            lowering.lowerType(halfRecord, NVVMTypeUse::HelperParameter, physicalHalfRecord)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            lowering.lowerType(halfRecord, NVVMTypeUse::Value, ordinaryHalfRecord)));
        SlangNVVMTypeHandle half = nullptr, bits = nullptr, expectedValue = nullptr,
                            expectedABI = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getFloatingPointType(scope.module, 16, half)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(provider.getIntegerType(scope.module, 16, bits)));
        SlangNVVMTypeHandle valueFields[] = {expectedGeneric, half};
        SlangNVVMTypeHandle abiFields[] = {expectedGeneric, bits};
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.getStructType(scope.module, valueFields, 2, expectedValue)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(provider.getStructType(scope.module, abiFields, 2, expectedABI)));
        SLANG_CHECK(ordinaryHalfRecord == expectedValue && physicalHalfRecord == expectedABI);
        SLANG_CHECK(ordinaryHalfRecord != physicalHalfRecord);
    }
    for (auto addressSpace :
         {AddressSpace::Generic, AddressSpace::GroupShared, AddressSpace::ThreadLocal})
    {
        auto denied = ir.createStructType();
        auto reference = ir.getPtrType(
            kIROp_PtrType,
            ir.getIntType(),
            AccessQualifier::ReadWrite,
            addressSpace,
            ir.getDefaultBufferLayoutType());
        ir.createStructField(denied, ir.createStructKey(), reference);
        SLANG_CHECK(!isNVVMSupportedParameterType(denied));
        SLANG_CHECK(!getNVVMCUDAValueLayout(&context.codeGen, denied, layout));
    }
}
