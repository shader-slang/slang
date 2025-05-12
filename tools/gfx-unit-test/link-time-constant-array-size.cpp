#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "gfx-util/shader-cursor.h"
#include "slang-gfx.h"
#include "unit-test/slang-unit-test.h"

using namespace gfx;

namespace gfx_test
{
static Slang::Result loadProgram(
    gfx::IDevice* device,
    Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
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
    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

void linkTimeConstantArraySizeTestImpl(IDevice* device, UnitTestContext* context)
{
    // Create transient heap
    Slang::ComPtr<ITransientResourceHeap> transientHeap;
    ITransientResourceHeap::Desc transientHeapDesc = {};
    transientHeapDesc.constantBufferSize = 4096;
    GFX_CHECK_CALL_ABORT(
        device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

    // Load and link program
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(
        loadProgram(device, shaderProgram, "main", "lib", "computeMain", slangReflection));

    // Create compute pipeline
    ComputePipelineStateDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<gfx::IPipelineState> pipelineState;
    GFX_CHECK_CALL_ABORT(
        device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

    // Create structured buffer for output
    const int N = 4; // This should match the constant in lib.slang

    // Create buffer for struct S with array of size N
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.sizeInBytes = sizeof(int) * N;
    bufferDesc.format = gfx::Format::Unknown;
    bufferDesc.elementSize = sizeof(int) * N;
    bufferDesc.allowedStates = ResourceStateSet(
        ResourceState::ShaderResource,
        ResourceState::UnorderedAccess,
        ResourceState::CopyDestination,
        ResourceState::CopySource);
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    // Initialize with zeros
    int initialData[N] = {0};
    ComPtr<IBufferResource> outputBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBufferResource(bufferDesc, initialData, outputBuffer.writeRef()));

    // Create UAV for the buffer
    ComPtr<IResourceView> outputView;
    IResourceView::Desc viewDesc = {};
    viewDesc.type = IResourceView::Type::UnorderedAccess;
    viewDesc.format = Format::Unknown;
    GFX_CHECK_CALL_ABORT(
        device->createBufferView(outputBuffer, nullptr, viewDesc, outputView.writeRef()));

    // Create parameter block for struct S
    int paramData[N] = {1, 2, 3, 4}; // Input values

    ComPtr<IBufferResource> paramBuffer;
    bufferDesc.sizeInBytes = sizeof(int) * N;
    bufferDesc.elementSize = sizeof(int) * N;
    GFX_CHECK_CALL_ABORT(
        device->createBufferResource(bufferDesc, paramData, paramBuffer.writeRef()));

    ComPtr<IResourceView> paramView;
    viewDesc.type = IResourceView::Type::ShaderResource;
    GFX_CHECK_CALL_ABORT(
        device->createBufferView(paramBuffer, nullptr, viewDesc, paramView.writeRef()));

    // Record and execute command buffer
    {
        ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
        auto queue = device->createCommandQueue(queueDesc);

        auto commandBuffer = transientHeap->createCommandBuffer();
        auto encoder = commandBuffer->encodeComputeCommands();

        auto rootObject = encoder->bindPipeline(pipelineState);

        ShaderCursor entryPointCursor(rootObject->getEntryPoint(0));

        // Bind output buffer
        entryPointCursor.getPath("b").setResource(outputView);

        // Bind parameter block
        entryPointCursor.getPath("p").setResource(paramView);

        encoder->dispatchCompute(1, 1, 1);
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);
        queue->waitOnHost();
    }

    // Expected results: each element is input * N
    // With N=4 and inputs [1,2,3,4], expected output is [4,8,12,16]
    compareComputeResult(device, outputBuffer, Slang::makeArray<int>(4, 6, 12, 16));
}

SLANG_UNIT_TEST(linkTimeConstantArraySizeD3D12)
{
    runTestImpl(linkTimeConstantArraySizeTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
}

SLANG_UNIT_TEST(linkTimeConstantArraySizeVulkan)
{
    runTestImpl(linkTimeConstantArraySizeTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
}

} // namespace gfx_test
