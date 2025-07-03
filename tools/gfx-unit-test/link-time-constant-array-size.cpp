#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include <slang-rhi/shader-cursor.h>
#include <slang-rhi.h>
#include "unit-test/slang-unit-test.h"

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

    // Load library module with constants
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

// Function to validate the array size in struct S
static void validateArraySizeInStruct(
    UnitTestContext* context,
    slang::ProgramLayout* slangReflection,
    int expectedSize)
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

    // Find the buffer resource 'b'
    bool foundBuffer = false;
    auto fieldCount = typeLayout->getFieldCount();

    for (unsigned int i = 0; i < fieldCount; i++)
    {
        auto fieldLayout = typeLayout->getFieldByIndex(i);
        const char* fieldName = fieldLayout->getName();

        if (fieldName && strcmp(fieldName, "b") == 0)
        {
            foundBuffer = true;

            // Get the type layout of the field
            auto fieldTypeLayout = fieldLayout->getTypeLayout();
            SLANG_CHECK_ABORT(fieldTypeLayout != nullptr);

            // Get the element type of the structured buffer
            auto elementTypeLayout = fieldTypeLayout->getElementTypeLayout();
            SLANG_CHECK_ABORT(elementTypeLayout != nullptr);

            // Check if it's a struct type
            auto elementKind = elementTypeLayout->getKind();
            SLANG_CHECK_ABORT(elementKind == slang::TypeReflection::Kind::Struct);

            // Get the field count of the struct
            auto structFieldCount = elementTypeLayout->getFieldCount();
            SLANG_CHECK_ABORT(structFieldCount >= 1);

            // Check for the 'xs' field
            bool foundXsField = false;
            for (unsigned int j = 0; j < structFieldCount; j++)
            {
                auto structField = elementTypeLayout->getFieldByIndex(j);
                const char* structFieldName = structField->getName();

                if (structFieldName && strcmp(structFieldName, "xs") == 0)
                {
                    foundXsField = true;

                    // Check that it's an array type
                    auto structFieldTypeLayout = structField->getTypeLayout();
                    auto structFieldTypeKind = structFieldTypeLayout->getKind();

                    SLANG_CHECK_ABORT(structFieldTypeKind == slang::TypeReflection::Kind::Array);

                    // Check the array size
                    auto arraySize = structFieldTypeLayout->getElementCount();
                    // 0 becuase we haven't resolved the constant
                    SLANG_CHECK_ABORT(arraySize == 0);

                    // 4 because we're resolving it
                    const auto resolvedArraySize =
                        structFieldTypeLayout->getElementCount(slangReflection);
                    SLANG_CHECK_ABORT(resolvedArraySize == expectedSize);

                    break;
                }
            }

            SLANG_CHECK_ABORT(foundXsField);
            break;
        }
    }

    SLANG_CHECK_ABORT(foundBuffer);
}


void linkTimeConstantArraySizeTestImpl(IDevice* device, UnitTestContext* context)
{
    // Load and link program
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    SLANG_CHECK_ABORT(loadProgram(
        device,
        shaderProgram,
        "link-time-constant-array-size-main",
        "link-time-constant-array-size-lib",
        "computeMain",
        slangReflection));

    // Check array size through reflection
    const int N = 4; // This should match the constant in lib.slang

    validateArraySizeInStruct(context, slangReflection, N);

    // Create compute pipeline
    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    SLANG_CHECK_ABORT(
        device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    // Create buffer for struct S with array of size N
    int32_t initialData[] = {1, 2, 3, 4};
    BufferDesc bufferDesc = {};
    bufferDesc.size = N * sizeof(int32_t);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(int32_t);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess | BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    SLANG_CHECK_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    // Record and execute command buffer
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipelineState);

        ShaderCursor rootCursor(rootObject);
        rootCursor.getPath("b").setBinding(Binding(numbersBuffer));

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    // Expected results: each element is input * N
    // With N=4 and inputs [1,2,3,4], expected output is [4,8,12,16]
    compareComputeResult(device, numbersBuffer, std::array{4, 8, 12, 16});
}

SLANG_UNIT_TEST(linkTimeConstantArraySizeD3D12)
{
    runTestImpl(linkTimeConstantArraySizeTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeConstantArraySizeVulkan)
{
    runTestImpl(linkTimeConstantArraySizeTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
