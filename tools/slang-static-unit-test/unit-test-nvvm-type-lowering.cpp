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
