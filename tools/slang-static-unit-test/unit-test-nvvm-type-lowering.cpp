// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Direct tests keep canonical value, physical helper ABI, and storage roles distinct.
#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm-type-lowering.h"

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
        SlangNVVMTypeHandle storage = nativeHalf;
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
            if (role == NVVMTypeUse::Storage && laneCount == 2)
            {
                // Bare half2 Storage is not currently admitted. Preserve that separate policy.
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(actual == nullptr);
                continue;
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            const auto expected =
                role == NVVMTypeUse::Storage ? storage
                : role == NVVMTypeUse::HelperParameter || role == NVVMTypeUse::HelperResult
                    ? boundary
                    : value;
            SLANG_CHECK(actual == expected);
        }
        for (auto forbidden : {NVVMTypeUse::EntryPointParameter, NVVMTypeUse::EntryPointResult})
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
