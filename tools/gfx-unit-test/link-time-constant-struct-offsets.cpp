/*
 * Test: Link-Time Constant Struct Field Offsets
 *
 * PURPOSE:
 * This test validates that Slang's reflection system correctly handles struct field
 * offsets when fields come after a link-time constant array. It ensures that:
 * 1. Before linking: Fields after unknown-size arrays return SLANG_UNKNOWN_SIZE offsets
 * 2. After linking: All field offsets are resolved to concrete values
 * 3. Field sizes are properly calculated before and after linking
 *
 * TESTED SCENARIO:
 * struct TestStruct {
 *     int a;                        // Always known offset: 0
 *     float b;                      // Always known offset: 4
 *     int dynamicArray[ARRAY_SIZE]; // Link-time constant sized array
 *     int c;                        // Unknown offset before linking
 *     float d;                      // Unknown offset before linking
 * };
 *
 * VALIDATION PHASES:
 * 1. Main module only (ARRAY_SIZE undefined):
 *    - dynamicArray has SLANG_UNKNOWN_SIZE element count and size
 *    - Fields c,d have SLANG_UNKNOWN_SIZE offsets
 * 2. After linking with lib module (ARRAY_SIZE = 3):
 *    - dynamicArray resolves to 12 bytes (3 × 4)
 *    - Fields c,d resolve to concrete offsets 20, 24
 */

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>
#include <slang-rhi.h>

using namespace rhi;

namespace gfx_test
{

// Helper function to get the TestStruct element type from buffer layout
static slang::TypeLayoutReflection* getTestStructLayout(
    slang::TypeLayoutReflection* globalTypeLayout)
{
    auto fieldCount = globalTypeLayout->getFieldCount();
    for (unsigned int i = 0; i < fieldCount; i++)
    {
        auto fieldLayout = globalTypeLayout->getFieldByIndex(i);
        const char* fieldName = fieldLayout->getName();
        if (fieldName && strcmp(fieldName, "buffer") == 0)
        {
            auto fieldTypeLayout = fieldLayout->getTypeLayout();
            return fieldTypeLayout->getElementTypeLayout();
        }
    }
    return nullptr;
}

// Unified function to validate struct field offsets and sizes
static void validateStructOffsets(
    UnitTestContext* context,
    slang::TypeLayoutReflection* globalTypeLayout,
    slang::ProgramLayout* programLayout, // null for before linking
    const char* const* expectedFieldNames,
    const size_t* expectedOffsets,
    const size_t* expectedSizes,
    unsigned int expectedFieldCount,
    bool expectArrayUnknownSize,
    const char* testPhase)
{
    SLANG_CHECK_ABORT(globalTypeLayout != nullptr);
    SLANG_CHECK_MSG(
        globalTypeLayout->getKind() == slang::TypeReflection::Kind::Struct,
        "Global scope is not a struct type");

    auto elementTypeLayout = getTestStructLayout(globalTypeLayout);
    SLANG_CHECK_MSG(elementTypeLayout != nullptr, "Could not find buffer 'buffer' in global scope");
    SLANG_CHECK_MSG(
        elementTypeLayout->getKind() == slang::TypeReflection::Kind::Struct,
        "Buffer element is not a struct type");

    auto structFieldCount = elementTypeLayout->getFieldCount();
    SLANG_CHECK_MSG(structFieldCount == expectedFieldCount, "Struct field count mismatch");

    for (unsigned int j = 0; j < structFieldCount; j++)
    {
        auto structField = elementTypeLayout->getFieldByIndex(j);
        const char* structFieldName = structField->getName();
        auto offset = structField->getOffset();
        auto fieldTypeLayout = structField->getTypeLayout();
        auto fieldKind = fieldTypeLayout->getKind();
        auto fieldSize = fieldTypeLayout->getSize();

        SLANG_CHECK_MSG(structFieldName != nullptr, "Struct field has no name");
        SLANG_CHECK_MSG(
            strcmp(structFieldName, expectedFieldNames[j]) == 0,
            "Unexpected field name");
        SLANG_CHECK_MSG(offset == expectedOffsets[j], "Field offset incorrect");
        SLANG_CHECK_MSG(fieldSize == expectedSizes[j], "Field size incorrect");

        // If this is the array field, check element count and size
        if (fieldKind == slang::TypeReflection::Kind::Array)
        {
            auto elementCount = fieldTypeLayout->getElementCount();

            if (expectArrayUnknownSize)
            {
                SLANG_CHECK_MSG(
                    elementCount == SLANG_UNKNOWN_SIZE,
                    "Array element count should be SLANG_UNKNOWN_SIZE before linking");
                SLANG_CHECK_MSG(
                    fieldSize == SLANG_UNKNOWN_SIZE,
                    "Array size should be SLANG_UNKNOWN_SIZE before linking");
            }
            else if (programLayout)
            {
                // After linking - check resolved element count
                auto resolvedElementCount = fieldTypeLayout->getElementCount(programLayout);
                SLANG_CHECK_MSG(
                    resolvedElementCount == 3,
                    "Array should have 3 elements after linking");
            }
        }
    }
}

void linkTimeConstantStructOffsetsTestImpl(IDevice* device, UnitTestContext* context)
{
    Slang::ComPtr<slang::ISession> slangSession;
    GFX_CHECK_CALL_ABORT(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    // Load main module ONLY (no lib module yet)
    slang::IModule* mainModule = slangSession->loadModule(
        "link-time-constant-struct-offsets-main",
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_CHECK_ABORT(mainModule != nullptr);

    // Find entry point
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    GFX_CHECK_CALL_ABORT(
        mainModule->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));

    // Compose program with ONLY main module and entry point (no lib module)
    Slang::List<slang::IComponentType*> mainOnlyComponentTypes;
    mainOnlyComponentTypes.add(mainModule);
    mainOnlyComponentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> mainOnlyProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        mainOnlyComponentTypes.getBuffer(),
        mainOnlyComponentTypes.getCount(),
        mainOnlyProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    GFX_CHECK_CALL_ABORT(result);

    // PHASE 1: Validate main-only reflection (before lib module is available)
    // Expected behavior: Fields after link-time array should have SLANG_UNKNOWN_SIZE offsets
    const char* expectedFieldNames[] = {"a", "b", "dynamicArray", "c", "d"};
    size_t beforeLinkingOffsets[] = {0, 4, 8, SLANG_UNKNOWN_SIZE, SLANG_UNKNOWN_SIZE};
    size_t beforeLinkingSizes[] =
        {4, 4, SLANG_UNKNOWN_SIZE, 4, 4}; // int, float, unknown array, int, float
    auto mainOnlyReflection = mainOnlyProgram->getLayout();
    validateStructOffsets(
        context,
        mainOnlyReflection->getGlobalParamsVarLayout()->getTypeLayout(),
        nullptr, // no program layout for before linking
        expectedFieldNames,
        beforeLinkingOffsets,
        beforeLinkingSizes,
        5,    // expected field count
        true, // expect array unknown size
        "before linking");

    // Now load the lib module
    slang::IModule* libModule = slangSession->loadModule(
        "link-time-constant-struct-offsets-lib",
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_CHECK_ABORT(libModule != nullptr);

    // Compose program from modules (main + lib + entry point)
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(mainModule);
    componentTypes.add(libModule);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    GFX_CHECK_CALL_ABORT(result);

    // Link program
    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    GFX_CHECK_CALL_ABORT(result);

    auto slangReflection = linkedProgram->getLayout();

    // Check struct offsets AFTER linking (should have resolved offsets)
    size_t afterLinkingOffsets[] = {0, 4, 8, 20, 24};
    size_t afterLinkingSizes[] = {4, 4, 12, 4, 4}; // int, float, int[3], int, float
    validateStructOffsets(
        context,
        slangReflection->getGlobalParamsVarLayout()->getTypeLayout(),
        slangReflection, // pass program layout for resolved checks
        expectedFieldNames,
        afterLinkingOffsets,
        afterLinkingSizes,
        5,     // expected field count
        false, // expect array size to be resolved
        "after linking");
}

SLANG_UNIT_TEST(linkTimeConstantStructOffsetsD3D12)
{
    runTestImpl(linkTimeConstantStructOffsetsTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeConstantStructOffsetsVulkan)
{
    runTestImpl(linkTimeConstantStructOffsetsTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
