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

// Function to validate struct field offsets
static void validateStructOffsets(
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
            SLANG_CHECK_MSG(structFieldCount == 4, "Struct should have 4 fields");

            // Check field offsets
            // struct TestStruct { int a; float b; int c; float d; }
            const char* expectedFieldNames[] = {"a", "b", "c", "d"};
            size_t expectedOffsets[] = {0, 4, 8, 12}; // int(4), float(4), int(4), float(4)

            for (unsigned int j = 0; j < structFieldCount; j++)
            {
                auto structField = elementTypeLayout->getFieldByIndex(j);
                const char* structFieldName = structField->getName();
                
                SLANG_CHECK_MSG(structFieldName != nullptr, "Struct field has no name");
                SLANG_CHECK_MSG(
                    strcmp(structFieldName, expectedFieldNames[j]) == 0,
                    "Unexpected field name");

                auto offset = structField->getOffset();
                SLANG_CHECK_MSG(
                    offset == expectedOffsets[j],
                    "Field offset does not match expected value");
            }

            break;
        }
    }

    SLANG_CHECK_MSG(foundBuffer, "Could not find buffer 'buffer' in global scope");
}

void structOffsetTestImpl(IDevice* device, UnitTestContext* context)
{
    // Load and link program
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        shaderProgram,
        "struct-offset-main",
        "struct-offset-lib",
        "computeMain",
        slangReflection));

    // Check struct offsets through reflection
    validateStructOffsets(context, slangReflection);

    // Create compute pipeline
    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    // Create buffer for TestStruct: {int a; float b; int c; float d;}
    uint32_t initialData[4] = {0, 0, 0, 0}; // 4 x 4-byte values = 16 bytes
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
    // Expected: TestStruct{1, 2.0f, 3, 4.0f} stored as bytes
    uint32_t expectedData[4];
    expectedData[0] = 1;
    float fb = 2.0f;
    memcpy(&expectedData[1], &fb, sizeof(float));
    expectedData[2] = 3;
    float fd = 4.0f;
    memcpy(&expectedData[3], &fd, sizeof(float));
    
    compareComputeResult(device, testBuffer, std::array{expectedData[0], expectedData[1], expectedData[2], expectedData[3]});
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