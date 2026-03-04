/*
 * Test: Link-Time Constant Nested Struct Field Offsets
 *
 * PURPOSE:
 * This test validates that Slang's reflection system correctly handles struct field
 * offsets when a nested struct contains a link-time constant array. It ensures that:
 * 1. Before linking: The nested struct has unknown size, affecting parent struct layout
 * 2. After linking: The nested struct size is resolved, and parent layout is concrete
 * 3. Fields after the nested struct get correct offsets based on resolved nested size
 *
 * TESTED SCENARIO:
 * struct NestedStruct {
 *     int x;                               // offset 0 within nested
 *     float y;                             // offset 4 within nested
 *     int nestedArray[NESTED_ARRAY_SIZE];  // Link-time constant array
 *     int z;                               // depends on array size
 * };
 *
 * struct ParentStruct {
 *     int a;                               // Always known offset: 0
 *     float b;                             // Always known offset: 4
 *     NestedStruct nested;                 // Unknown size before linking
 *     int c;                               // Unknown offset before linking
 *     float d;                             // Unknown offset before linking
 * };
 *
 * VALIDATION PHASES:
 * 1. Main module only (NESTED_ARRAY_SIZE undefined):
 *    - nested has SLANG_UNKNOWN_SIZE size
 *    - Fields c,d have SLANG_UNKNOWN_SIZE offsets
 * 2. After linking with lib module (NESTED_ARRAY_SIZE = 3):
 *    - NestedStruct resolves to 24 bytes (4+4+12+4)
 *    - nested field has concrete size 24
 *    - Fields c,d resolve to concrete offsets 32, 36
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

// Helper function to get the ParentStruct element type from buffer layout
static slang::TypeLayoutReflection* getParentStructLayout(
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
    bool expectNestedArrayUnknownSize,
    const char* testPhase)
{
    SLANG_CHECK_ABORT(globalTypeLayout != nullptr);
    SLANG_CHECK_MSG(
        globalTypeLayout->getKind() == slang::TypeReflection::Kind::Struct,
        "Global scope is not a struct type");

    auto elementTypeLayout = getParentStructLayout(globalTypeLayout);
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

        // If this is the nested struct field, check its behavior
        if (fieldKind == slang::TypeReflection::Kind::Struct &&
            strcmp(structFieldName, "nested") == 0)
        {
            if (expectNestedArrayUnknownSize)
            {
                SLANG_CHECK_MSG(
                    fieldSize == SLANG_UNKNOWN_SIZE,
                    "Nested struct size should be SLANG_UNKNOWN_SIZE before linking");
            }
            else if (programLayout)
            {
                // After linking - the nested struct should have a concrete size
                SLANG_CHECK_MSG(
                    fieldSize > 0 && fieldSize != SLANG_UNKNOWN_SIZE,
                    "Nested struct should have concrete size after linking");
            }
        }
    }
}

void linkTimeConstantNestedStructOffsetsTestImpl(IDevice* device, UnitTestContext* context)
{
    Slang::ComPtr<slang::ISession> slangSession;
    GFX_CHECK_CALL_ABORT(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    // Load main module ONLY (no lib module yet)
    slang::IModule* mainModule = slangSession->loadModule(
        "link-time-constant-nested-struct-offsets-main",
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

    // Check struct offsets with ONLY main module (nested struct with link-time array should be
    // unknown)
    const char* expectedFieldNames[] = {"a", "b", "nested", "c", "d"};
    size_t beforeLinkingOffsets[] = {0, 4, 8, SLANG_UNKNOWN_SIZE, SLANG_UNKNOWN_SIZE};
    size_t beforeLinkingSizes[] =
        {4, 4, SLANG_UNKNOWN_SIZE, 4, 4}; // int, float, unknown nested struct, int, float
    auto mainOnlyReflection = mainOnlyProgram->getLayout();
    validateStructOffsets(
        context,
        mainOnlyReflection->getGlobalParamsVarLayout()->getTypeLayout(),
        nullptr, // no program layout for before linking
        expectedFieldNames,
        beforeLinkingOffsets,
        beforeLinkingSizes,
        5,    // expected field count
        true, // expect nested struct with array unknown size
        "before linking");

    // Now load the lib module
    slang::IModule* libModule = slangSession->loadModule(
        "link-time-constant-nested-struct-offsets-lib",
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
    // ParentStruct { int a; float b; NestedStruct nested; int c; float d; }
    // NestedStruct { int x; float y; int nestedArray[3]; int z; } = 4 + 4 + 12 + 4 = 24 bytes
    size_t afterLinkingOffsets[] = {0, 4, 8, 32, 36};
    size_t afterLinkingSizes[] = {4, 4, 24, 4, 4}; // int, float, NestedStruct(24 bytes), int, float
    validateStructOffsets(
        context,
        slangReflection->getGlobalParamsVarLayout()->getTypeLayout(),
        slangReflection, // pass program layout for resolved checks
        expectedFieldNames,
        afterLinkingOffsets,
        afterLinkingSizes,
        5,     // expected field count
        false, // expect nested struct size to be resolved
        "after linking");
}

SLANG_UNIT_TEST(linkTimeConstantNestedStructOffsetsD3D12)
{
    runTestImpl(linkTimeConstantNestedStructOffsetsTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeConstantNestedStructOffsetsVulkan)
{
    runTestImpl(linkTimeConstantNestedStructOffsetsTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test