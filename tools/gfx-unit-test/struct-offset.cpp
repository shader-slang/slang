#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>
#include <cstring>

using namespace rhi;

namespace gfx_test
{
static Slang::Result loadProgram(
    IDevice* device,
    Slang::ComPtr<IShaderProgram>& outShaderProgram,
    const char* mainModuleName,
    const char* libModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection)
{
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    // Load main module
    slang::IModule* mainModule =
        slangSession->loadModule(mainModuleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!mainModule)
        return SLANG_FAIL;

    // Load library module (may be empty for basic test)
    slang::IModule* libModule = slangSession->loadModule(libModuleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!libModule)
        return SLANG_FAIL;

    // Find entry point
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        mainModule->findEntryPointByName(entryPointName, computeEntryPoint.writeRef()));

    // Compose program from modules
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(mainModule);
    componentTypes.add(libModule);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    // Link program
    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    composedProgram = linkedProgram;
    slangReflection = composedProgram->getLayout();

    // Create shader program
    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createShaderProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

// Function to validate struct field offsets before linking (should check SLANG_UNKNOWN_SIZE)
static void validateStructOffsetsBeforeLinking(
    UnitTestContext* context,
    slang::IComponentType* unlinkedProgram)
{
    auto prelinkReflection = unlinkedProgram->getLayout();
    SLANG_CHECK_ABORT(prelinkReflection != nullptr);

    // Get the global scope layout
    auto globalScope = prelinkReflection->getGlobalParamsVarLayout();
    SLANG_CHECK_ABORT(globalScope != nullptr);

    auto typeLayout = globalScope->getTypeLayout();
    SLANG_CHECK_ABORT(typeLayout != nullptr);

    // Check if the global scope is a struct type
    auto kind = typeLayout->getKind();
    SLANG_CHECK_ABORT(kind == slang::TypeReflection::Kind::Struct);

    // Find the buffer resource 'buffer'
    bool foundBuffer = false;
    auto fieldCount = typeLayout->getFieldCount();

    for (unsigned int i = 0; i < fieldCount; i++)
    {
        auto fieldLayout = typeLayout->getFieldByIndex(i);
        const char* fieldName = fieldLayout->getName();

        if (fieldName && strcmp(fieldName, "buffer") == 0)
        {
            foundBuffer = true;

            // Get the element type of the structured buffer
            auto fieldTypeLayout = fieldLayout->getTypeLayout();
            auto elementTypeLayout = fieldTypeLayout->getElementTypeLayout();
            SLANG_CHECK_MSG(
                elementTypeLayout != nullptr,
                "Structured buffer has no element type layout");

            // Check if it's a struct type
            auto elementKind = elementTypeLayout->getKind();
            SLANG_CHECK_MSG(
                elementKind == slang::TypeReflection::Kind::Struct,
                "Buffer element is not a struct type");

            // Get the field count of the struct
            auto structFieldCount = elementTypeLayout->getFieldCount();
            SLANG_CHECK_MSG(structFieldCount == 5, "Struct should have 5 fields");

            // Check field offsets - fields after dynamicArray should have SLANG_UNKNOWN_SIZE
            // struct TestStruct { int a; float b; int dynamicArray[ARRAY_SIZE]; int c; float d; }
            const char* expectedFieldNames[] = {"a", "b", "dynamicArray", "c", "d"};
            
            printf("=== BEFORE LINKING DEBUG INFO ===\n");
            printf("Struct has %u fields\n", structFieldCount);
            printf("SLANG_UNKNOWN_SIZE = %zu (0x%zx)\n", (size_t)SLANG_UNKNOWN_SIZE, (size_t)SLANG_UNKNOWN_SIZE);
            
            for (unsigned int j = 0; j < structFieldCount; j++)
            {
                auto structField = elementTypeLayout->getFieldByIndex(j);
                const char* structFieldName = structField->getName();
                auto offset = structField->getOffset();
                auto fieldTypeLayout = structField->getTypeLayout();
                auto fieldKind = fieldTypeLayout->getKind();
                
                printf("Field %u: name='%s', offset=%zu, kind=%d\n", 
                       j, structFieldName ? structFieldName : "NULL", offset, (int)fieldKind);
                
                // If this is an array field, check its element count
                if (fieldKind == slang::TypeReflection::Kind::Array)
                {
                    auto elementCount = fieldTypeLayout->getElementCount();
                    printf("  ARRAY FIELD ANALYSIS:\n");
                    printf("    elementCount = %zu\n", elementCount);
                    printf("    SLANG_UNKNOWN_SIZE = %zu\n", (size_t)SLANG_UNKNOWN_SIZE);
                    printf("    elementCount == SLANG_UNKNOWN_SIZE? %s\n", 
                           (elementCount == SLANG_UNKNOWN_SIZE) ? "YES" : "NO");
                    
                    // This is the key test - array element count should be SLANG_UNKNOWN_SIZE before linking
                    SLANG_CHECK_MSG(
                        elementCount == SLANG_UNKNOWN_SIZE,
                        "Array element count should be SLANG_UNKNOWN_SIZE before linking");
                        
                    // Also check the size in bytes of the array
                    auto arraySize = fieldTypeLayout->getSize();
                    printf("    Array size in bytes: %zu\n", arraySize);
                    if (arraySize == SLANG_UNKNOWN_SIZE)
                    {
                        printf("    Array byte size is SLANG_UNKNOWN_SIZE - this makes sense!\n");
                    }
                    else
                    {
                        printf("    Array byte size is concrete: %zu bytes - how is this possible?\n", arraySize);
                    }
                }
                
                SLANG_CHECK_MSG(structFieldName != nullptr, "Struct field has no name");
                SLANG_CHECK_MSG(
                    strcmp(structFieldName, expectedFieldNames[j]) == 0,
                    "Unexpected field name");

                // Fields before the array should have known offsets
                if (j == 0) // int a
                {
                    printf("  Expected offset 0, got %zu\n", offset);
                    SLANG_CHECK_MSG(offset == 0, "Field 'a' offset should be 0");
                }
                else if (j == 1) // float b  
                {
                    printf("  Expected offset 4, got %zu\n", offset);
                    SLANG_CHECK_MSG(offset == 4, "Field 'b' offset should be 4");
                }
                else if (j == 2) // dynamicArray
                {
                    printf("  Expected offset 8, got %zu\n", offset);
                    SLANG_CHECK_MSG(offset == 8, "Field 'dynamicArray' offset should be 8");
                }
                else // Fields c and d come after the dynamicArray
                {
                    printf("  FIELD AFTER LINK-TIME ARRAY ANALYSIS:\n");
                    printf("    Field '%s' offset = %zu\n", structFieldName, offset);
                    printf("    SLANG_UNKNOWN_SIZE = %zu\n", (size_t)SLANG_UNKNOWN_SIZE);
                    printf("    offset == SLANG_UNKNOWN_SIZE? %s\n", 
                           (offset == SLANG_UNKNOWN_SIZE) ? "YES" : "NO");
                    
                    if (offset == SLANG_UNKNOWN_SIZE)
                    {
                        printf("    ✓ CORRECT: Field after unknown-size array has SLANG_UNKNOWN_SIZE offset\n");
                    }
                    else
                    {
                        printf("    ✗ UNEXPECTED: Field after unknown-size array has concrete offset %zu\n", offset);
                        printf("    ❓ How can Slang calculate an offset when the preceding array size is unknown?\n");
                        
                        // Let's check if somehow the array size got resolved
                        auto prevField = elementTypeLayout->getFieldByIndex(j-1);
                        if (prevField)
                        {
                            auto prevFieldType = prevField->getTypeLayout();
                            if (prevFieldType->getKind() == slang::TypeReflection::Kind::Array)
                            {
                                auto prevElementCount = prevFieldType->getElementCount();
                                auto prevSize = prevFieldType->getSize();
                                printf("    Previous array field: elementCount=%zu, size=%zu\n", 
                                       prevElementCount, prevSize);
                                if (prevSize != SLANG_UNKNOWN_SIZE)
                                {
                                    printf("    ❓ Previous array has concrete size, which explains this offset\n");
                                }
                            }
                        }
                    }
                }
            }
            printf("=== END BEFORE LINKING DEBUG ===\n");

            break;
        }
    }

    SLANG_CHECK_MSG(foundBuffer, "Could not find buffer 'buffer' in global scope");
}

// Function to validate struct field offsets after linking
static void validateStructOffsetsAfterLinking(
    UnitTestContext* context,
    slang::ProgramLayout* slangReflection)
{
    // Check reflection is available
    SLANG_CHECK_ABORT(slangReflection != nullptr);

    // Get the global scope layout
    auto globalScope = slangReflection->getGlobalParamsVarLayout();
    SLANG_CHECK_ABORT(globalScope != nullptr);

    auto typeLayout = globalScope->getTypeLayout();
    SLANG_CHECK_ABORT(typeLayout != nullptr);

    // Check if the global scope is a struct type
    auto kind = typeLayout->getKind();
    SLANG_CHECK_ABORT(kind == slang::TypeReflection::Kind::Struct);

    // Find the buffer resource 'buffer'
    bool foundBuffer = false;
    auto fieldCount = typeLayout->getFieldCount();

    for (unsigned int i = 0; i < fieldCount; i++)
    {
        auto fieldLayout = typeLayout->getFieldByIndex(i);
        const char* fieldName = fieldLayout->getName();

        if (fieldName && strcmp(fieldName, "buffer") == 0)
        {
            foundBuffer = true;

            // Get the type layout of the field
            auto fieldTypeLayout = fieldLayout->getTypeLayout();
            SLANG_CHECK_MSG(fieldTypeLayout != nullptr, "Field has no type layout");

            // Get the element type of the structured buffer
            auto elementTypeLayout = fieldTypeLayout->getElementTypeLayout();
            SLANG_CHECK_MSG(
                elementTypeLayout != nullptr,
                "Structured buffer has no element type layout");
            
            // Check if it's a struct type
            auto elementKind = elementTypeLayout->getKind();
            SLANG_CHECK_MSG(
                elementKind == slang::TypeReflection::Kind::Struct,
                "Buffer element is not a struct type");

            // Get the field count of the struct
            auto structFieldCount = elementTypeLayout->getFieldCount();
            SLANG_CHECK_MSG(structFieldCount == 5, "Struct should have 5 fields");

            // Check field offsets after linking - all should have resolved offsets
            // struct TestStruct { int a; float b; int dynamicArray[3]; int c; float d; }
            const char* expectedFieldNames[] = {"a", "b", "dynamicArray", "c", "d"};
            size_t expectedOffsets[] = {0, 4, 8, 20, 24}; // int(4), float(4), int[3](12), int(4), float(4)

            printf("\n=== AFTER LINKING DEBUG INFO ===\n");
            printf("Struct has %u fields\n", structFieldCount);
            printf("SLANG_UNKNOWN_SIZE = %zu (0x%zx)\n", (size_t)SLANG_UNKNOWN_SIZE, (size_t)SLANG_UNKNOWN_SIZE);

            for (unsigned int j = 0; j < structFieldCount; j++)
            {
                auto structField = elementTypeLayout->getFieldByIndex(j);
                const char* structFieldName = structField->getName();
                auto offset = structField->getOffset();
                auto fieldTypeLayout = structField->getTypeLayout();
                auto fieldKind = fieldTypeLayout->getKind();
                
                printf("Field %u: name='%s', offset=%zu (expected=%zu), kind=%d\n", 
                       j, structFieldName ? structFieldName : "NULL", 
                       offset, expectedOffsets[j], (int)fieldKind);
                
                // If this is an array field, check its element count
                if (fieldKind == slang::TypeReflection::Kind::Array)
                {
                    auto elementCount = fieldTypeLayout->getElementCount();
                    auto resolvedElementCount = fieldTypeLayout->getElementCount(slangReflection);
                    auto arraySize = fieldTypeLayout->getSize();
                    
                    printf("  ARRAY FIELD AFTER LINKING:\n");
                    printf("    Regular elementCount = %zu\n", elementCount);
                    printf("    Resolved elementCount = %zu\n", resolvedElementCount);
                    printf("    Array size in bytes = %zu\n", arraySize);
                    printf("    SLANG_UNKNOWN_SIZE = %zu\n", (size_t)SLANG_UNKNOWN_SIZE);
                    
                    printf("    Regular elementCount == SLANG_UNKNOWN_SIZE? %s\n", 
                           (elementCount == SLANG_UNKNOWN_SIZE) ? "YES (WRONG!)" : "NO (CORRECT)");
                    printf("    Resolved elementCount == SLANG_UNKNOWN_SIZE? %s\n", 
                           (resolvedElementCount == SLANG_UNKNOWN_SIZE) ? "YES (WRONG!)" : "NO (CORRECT)");
                    printf("    Array size == SLANG_UNKNOWN_SIZE? %s\n", 
                           (arraySize == SLANG_UNKNOWN_SIZE) ? "YES (WRONG!)" : "NO (CORRECT)");
                    
                    // After linking, resolved count should not be SLANG_UNKNOWN_SIZE
                    SLANG_CHECK_MSG(
                        resolvedElementCount != SLANG_UNKNOWN_SIZE,
                        "After linking, resolved element count should not be SLANG_UNKNOWN_SIZE");
                    
                    SLANG_CHECK_MSG(
                        resolvedElementCount == 3,
                        "After linking, resolved element count should be 3");
                        
                    if (elementCount == SLANG_UNKNOWN_SIZE)
                    {
                        printf("    ✗ PROBLEM: Regular elementCount still SLANG_UNKNOWN_SIZE after linking!\n");
                    }
                    else
                    {
                        printf("    ✓ GOOD: Regular elementCount resolved to %zu after linking\n", elementCount);
                    }
                }
                
                SLANG_CHECK_MSG(structFieldName != nullptr, "Struct field has no name");
                SLANG_CHECK_MSG(
                    strcmp(structFieldName, expectedFieldNames[j]) == 0,
                    "Unexpected field name");

                SLANG_CHECK_MSG(
                    offset == expectedOffsets[j],
                    "Field offset does not match expected value after linking");
                    
                // Check field offsets after linking
                printf("  FIELD OFFSET AFTER LINKING:\n");
                printf("    offset = %zu, expected = %zu\n", offset, expectedOffsets[j]);
                printf("    offset == SLANG_UNKNOWN_SIZE? %s\n", 
                       (offset == SLANG_UNKNOWN_SIZE) ? "YES (WRONG!)" : "NO (CORRECT)");
                
                if (offset == SLANG_UNKNOWN_SIZE)
                {
                    printf("    ✗ PROBLEM: Field has SLANG_UNKNOWN_SIZE offset after linking!\n");
                }
                else if (offset == expectedOffsets[j])
                {
                    printf("    ✓ GOOD: Field has correct resolved offset\n");
                }
                else
                {
                    printf("    ✗ PROBLEM: Field has wrong offset %zu, expected %zu\n", offset, expectedOffsets[j]);
                }

                // Additionally check that the dynamicArray has the correct element count
                if (j == 2) // dynamicArray field
                {
                    auto arrayTypeLayout = structField->getTypeLayout();
                    auto elementCount = arrayTypeLayout->getElementCount();
                    auto resolvedElementCount = arrayTypeLayout->getElementCount(slangReflection);
                    printf("  Dynamic array: elementCount=%zu, resolvedCount=%zu\n", elementCount, resolvedElementCount);
                    
                    // After linking, the resolved element count should be 3
                    SLANG_CHECK_MSG(
                        resolvedElementCount == 3,
                        "Dynamic array should have 3 elements after linking (resolved)");
                        
                    // Document current behavior: regular element count may still be SLANG_UNKNOWN_SIZE
                    if (elementCount == SLANG_UNKNOWN_SIZE)
                    {
                        printf("  NOTE: Regular element count still SLANG_UNKNOWN_SIZE after linking\n");
                        printf("  NOTE: Use getElementCount(layout) for resolved value\n");
                    }
                    else
                    {
                        printf("  NOTE: Regular element count resolved to %zu after linking\n", elementCount);
                        SLANG_CHECK_MSG(
                            elementCount == 3,
                            "Dynamic array element count should be 3 after linking if resolved");
                    }
                }
            }
            printf("=== END AFTER LINKING DEBUG ===\n");

            break;
        }
    }

    SLANG_CHECK_MSG(foundBuffer, "Could not find buffer 'buffer' in global scope");
}

void structOffsetTestImpl(IDevice* device, UnitTestContext* context)
{
    Slang::ComPtr<slang::ISession> slangSession;
    GFX_CHECK_CALL_ABORT(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    // Load modules
    slang::IModule* mainModule = slangSession->loadModule("struct-offset-main", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_CHECK_ABORT(mainModule != nullptr);

    slang::IModule* libModule = slangSession->loadModule("struct-offset-lib", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_CHECK_ABORT(libModule != nullptr);

    // Find entry point
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    GFX_CHECK_CALL_ABORT(mainModule->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));

    // Compose program from modules (before linking)
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(mainModule);
    componentTypes.add(libModule);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    GFX_CHECK_CALL_ABORT(result);

    // Check struct offsets BEFORE linking (should have SLANG_UNKNOWN_SIZE for fields after link-time array)
    validateStructOffsetsBeforeLinking(context, composedProgram);

    // Link program
    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    GFX_CHECK_CALL_ABORT(result);

    auto slangReflection = linkedProgram->getLayout();

    // Check struct offsets AFTER linking (should have resolved offsets)
    validateStructOffsetsAfterLinking(context, slangReflection);

    // Create shader program
    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram.get();
    auto shaderProgram = device->createShaderProgram(programDesc);
    SLANG_CHECK_ABORT(shaderProgram != nullptr);

    // Create compute pipeline
    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    // Create buffer for TestStruct: {int a; float b; int dynamicArray[3]; int c; float d;}
    uint32_t initialData[7] = {0, 0, 0, 0, 0, 0, 0}; // 7 x 4-byte values = 28 bytes
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(initialData);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(initialData);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> testBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, testBuffer.writeRef()));

    // Record and execute command buffer
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipelineState);

        ShaderCursor rootCursor(rootObject);
        rootCursor.getPath("buffer").setBinding(Binding(testBuffer));

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    // Verify results by checking the buffer content
    // Expected: TestStruct{1, 2.0f, {10,11,12}, 3, 4.0f} stored as bytes
    uint32_t expectedData[7];
    expectedData[0] = 1;                    // int a
    float fb = 2.0f;
    memcpy(&expectedData[1], &fb, sizeof(float)); // float b
    expectedData[2] = 10;                   // dynamicArray[0]
    expectedData[3] = 11;                   // dynamicArray[1] 
    expectedData[4] = 12;                   // dynamicArray[2]
    expectedData[5] = 3;                    // int c
    float fd = 4.0f;
    memcpy(&expectedData[6], &fd, sizeof(float)); // float d
    
    compareComputeResult(device, testBuffer, std::array{expectedData[0], expectedData[1], expectedData[2], expectedData[3], expectedData[4], expectedData[5], expectedData[6]});
}

SLANG_UNIT_TEST(structOffsetD3D12)
{
    runTestImpl(structOffsetTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structOffsetVulkan)
{
    runTestImpl(structOffsetTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test