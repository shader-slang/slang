// main.cpp
#include "slang-com-ptr.h"
#include "slang.h"

#include <string>
using Slang::ComPtr;

#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "gpu-printing.h"
#include "platform/window.h"
#include "slang-rhi.h"

#include <slang-rhi/shader-cursor.h>

using namespace rhi;

static const ExampleResources resourceBase("gpu-printing");

ComPtr<slang::ISession> createSlangSession(IDevice* device)
{
    ComPtr<slang::ISession> slangSession = device->getSlangSession();
    return slangSession;
}

ComPtr<slang::IModule> compileShaderModuleFromFile(
    slang::ISession* slangSession,
    char const* filePath)
{
    ComPtr<slang::IModule> slangModule;
    ComPtr<slang::IBlob> diagnosticBlob;
    Slang::String path = resourceBase.resolveResource(filePath);
    slangModule = slangSession->loadModule(path.getBuffer(), diagnosticBlob.writeRef());
    diagnoseIfNeeded(diagnosticBlob);

    return slangModule;
}

struct ExampleProgram : public TestBase
{
    int gWindowWidth = 640;
    int gWindowHeight = 480;

    ComPtr<IDevice> gDevice;

    ComPtr<slang::ISession> gSlangSession;
    ComPtr<slang::IModule> gSlangModule;
    ComPtr<IShaderProgram> gProgram;

    ComPtr<IComputePipeline> gPipelineState;

    Slang::Dictionary<int, std::string> gHashedStrings;

    GPUPrinting gGPUPrinting;

    ComPtr<IShaderProgram> loadComputeProgram(
        slang::IModule* slangModule,
        char const* entryPointName)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        slangModule->findEntryPointByName(entryPointName, entryPoint.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        entryPoint->link(linkedProgram.writeRef());

        if (isTestMode())
        {
            printEntrypointHashes(1, 1, linkedProgram);
        }

        gGPUPrinting.loadStrings(linkedProgram->getLayout());

        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram;

        auto shaderProgram = gDevice->createShaderProgram(programDesc);

        return shaderProgram;
    }

    Result execute(int argc, char* argv[])
    {
        parseOption(argc, argv);
        DeviceDesc deviceDesc;
        gDevice = getRHI()->createDevice(deviceDesc);
        if (!gDevice)
            return SLANG_FAIL;

        Slang::String path = resourceBase.resolveResource("kernels.slang");

        gSlangSession = createSlangSession(gDevice);
        gSlangModule = compileShaderModuleFromFile(gSlangSession, path.getBuffer());
        if (!gSlangModule)
            return SLANG_FAIL;

        gProgram = loadComputeProgram(gSlangModule, "computeMain");
        if (!gProgram)
            return SLANG_FAIL;

        ComputePipelineDesc desc;
        desc.program = gProgram;
        auto pipelineState = gDevice->createComputePipeline(desc);
        if (!pipelineState)
            return SLANG_FAIL;

        gPipelineState = pipelineState;

        size_t printBufferSize = 4 * 1024; // use a small-ish (4KB) buffer for print output

        BufferDesc printBufferDesc = {};
        printBufferDesc.size = printBufferSize;
        printBufferDesc.elementSize = sizeof(uint32_t);
        printBufferDesc.usage =
            BufferUsage::UnorderedAccess | BufferUsage::CopySource | BufferUsage::CopyDestination;
        printBufferDesc.memoryType = MemoryType::DeviceLocal;
        auto printBuffer = gDevice->createBuffer(printBufferDesc);

        auto queue = gDevice->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto computeEncoder = commandEncoder->beginComputePass();
        auto rootShaderObject = computeEncoder->bindPipeline(gPipelineState);
        auto cursor = ShaderCursor(rootShaderObject);
        cursor["gPrintBuffer"].setBinding(printBuffer);

        computeEncoder->dispatchCompute(1, 1, 1);

        computeEncoder->end();
        queue->submit(commandEncoder->finish());

        ComPtr<ISlangBlob> blob;
        gDevice->readBuffer(printBuffer, 0, printBufferSize, blob.writeRef());

        gGPUPrinting.processGPUPrintCommands(blob->getBufferPointer(), printBufferSize);

        return SLANG_OK;
    }
};

int exampleMain(int argc, char** argv)
{
    ExampleProgram app;
    if (SLANG_FAILED(app.execute(argc, argv)))
    {
        return -1;
    }
    return 0;
}
