#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

// Test that a type can be used to serve multiple link-time type requirements.

namespace gfx_test
{
static Slang::Result loadProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    slang::ProgramLayout*& slangReflection,
    bool linkSpecialization = false)
{
    const char* moduleInterfaceSrc = R"(
            interface IFoo { int getFoo(); }
            interface IBar { int getBar(); }
            struct SimpleImpl : IFoo, IBar
            {
                int getFoo() { return 10; }
                int getBar() { return 20; }
            }
        )";
    const char* module0Src = R"(
            import ifoo;
            extern struct Foo : IFoo;
            extern struct Bar : IBar;
            uniform Foo gFoo;
            uniform Bar gBar;
            [numthreads(1,1,1)]
            void computeMain(uniform RWStructuredBuffer<int> buffer)
            {
                buffer[0] = gFoo.getFoo() + gBar.getBar();
            }
        )";
    const char* module1Src = R"(
            import ifoo;
            export struct Foo : IFoo = SimpleImpl;
            export struct Bar : IBar = SimpleImpl;
        )";
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    auto moduleInterfaceBlob =
        Slang::UnownedRawBlob::create(moduleInterfaceSrc, strlen(moduleInterfaceSrc));
    auto module0Blob = Slang::UnownedRawBlob::create(module0Src, strlen(module0Src));
    auto module1Blob = Slang::UnownedRawBlob::create(module1Src, strlen(module1Src));
    slang::IModule* moduleInterface =
        slangSession->loadModuleFromSource("ifoo", "ifoo.slang", moduleInterfaceBlob);
    slang::IModule* module0 = slangSession->loadModuleFromSource("module0", "path0", module0Blob);
    slang::IModule* module1 = slangSession->loadModuleFromSource("module1", "path1", module1Blob);
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module0->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));

    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(moduleInterface);
    componentTypes.add(module0);
    if (linkSpecialization)
        componentTypes.add(module1);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    composedProgram = linkedProgram;
    slangReflection = composedProgram->getLayout();

    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createShaderProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

void linkTimeTypeMultiUseTestImpl(IDevice* device, UnitTestContext* context)
{
    // Create pipeline without both modules linked, specifying both Foo and Bar to be SimpleImpl.
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadProgram(device, shaderProgram, slangReflection, true));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 0.0f, 0.0f, 0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    auto queue = device->getQueue(QueueType::Graphics);

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto commandEncoder = queue->createCommandEncoder();
        auto computePassEncoder = commandEncoder->beginComputePass();

        auto rootObject = computePassEncoder->bindPipeline(pipelineState);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        computePassEncoder->dispatchCompute(1, 1, 1);
        computePassEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{30});
}

SLANG_UNIT_TEST(linkTimeTypeMultiUseD3D12)
{
    runTestImpl(linkTimeTypeMultiUseTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeTypeMultiUseVulkan)
{
    runTestImpl(linkTimeTypeMultiUseTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
