// main.cpp

#include <slang.h>

#include <slang-com-ptr.h>
using Slang::ComPtr;

#include "slang-gfx.h"
#include "tools/platform/window.h"
#include "source/core/slang-basic.h"
using namespace gfx;

#include <string>

#include "gpu-printing.h"

ComPtr<slang::ISession> createSlangSession(gfx::IDevice* device)
{
    ComPtr<slang::ISession> slangSession = device->getSlangSession();
    return slangSession;
}

ComPtr<slang::IModule> compileShaderModuleFromFile(slang::ISession* slangSession, char const* filePath)
{
    SlangCompileRequest* slangRequest = nullptr;
    slangSession->createCompileRequest(&slangRequest);

    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, filePath);
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, filePath);

    const SlangResult compileRes = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        printf("%s", diagnostics);
    }

    if(SLANG_FAILED(compileRes))
    {
        spDestroyCompileRequest(slangRequest);
        return ComPtr<slang::IModule>();
    }

    ComPtr<slang::IModule> slangModule;
    spCompileRequest_getModule(slangRequest, translationUnitIndex, slangModule.writeRef());
    return slangModule;
}

struct ExampleProgram
{
int gWindowWidth = 640;
int gWindowHeight = 480;

ComPtr<gfx::IDevice>      gDevice;

ComPtr<slang::ISession> gSlangSession;
ComPtr<slang::IModule>   gSlangModule;
ComPtr<gfx::IShaderProgram> gProgram;

ComPtr<gfx::IPipelineState> gPipelineState;

Slang::Dictionary<int, std::string> gHashedStrings;

GPUPrinting gGPUPrinting;

ComPtr<gfx::IShaderProgram> loadComputeProgram(slang::IModule* slangModule, char const* entryPointName)
{
    ComPtr<slang::IEntryPoint> entryPoint;
    slangModule->findEntryPointByName(entryPointName, entryPoint.writeRef());

    ComPtr<slang::IComponentType> linkedProgram;
    entryPoint->link(linkedProgram.writeRef());

    gGPUPrinting.loadStrings(linkedProgram->getLayout());

    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.slangProgram = linkedProgram;

    auto shaderProgram = gDevice->createProgram(programDesc);

    return shaderProgram;
}

Result execute()
{
    IDevice::Desc deviceDesc;
    Result res = gfxCreateDevice(&deviceDesc, gDevice.writeRef());
    if(SLANG_FAILED(res)) return res;

    gSlangSession = createSlangSession(gDevice);
    gSlangModule = compileShaderModuleFromFile(gSlangSession, "kernels.slang");

    gProgram = loadComputeProgram(gSlangModule, "computeMain");
    if(!gProgram) return SLANG_FAIL;

    ComputePipelineStateDesc desc;
    desc.program = gProgram;
    auto pipelineState = gDevice->createComputePipelineState(desc);
    if(!pipelineState) return SLANG_FAIL;

    gPipelineState = pipelineState;

    size_t printBufferSize = 4 * 1024; // use a small-ish (4KB) buffer for print output

    IBufferResource::Desc printBufferDesc;
    printBufferDesc.init(printBufferSize);
    printBufferDesc.elementSize = sizeof(uint32_t);
    printBufferDesc.cpuAccessFlags = IResource::AccessFlag::Read; // | Resource::AccessFlag::Write;
    auto printBuffer =
        gDevice->createBufferResource(IResource::Usage::UnorderedAccess, printBufferDesc);

    IResourceView::Desc printBufferViewDesc;
    printBufferViewDesc.type = IResourceView::Type::UnorderedAccess;
    auto printBufferView = gDevice->createBufferView(printBuffer, printBufferViewDesc);

    ITransientResourceHeap::Desc transientResourceHeapDesc = {};
    transientResourceHeapDesc.constantBufferSize = 256;
    auto transientHeap = gDevice->createTransientResourceHeap(transientResourceHeapDesc);

    ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
    auto queue = gDevice->createCommandQueue(queueDesc);
    auto commandBuffer = transientHeap->createCommandBuffer();
    auto encoder = commandBuffer->encodeComputeCommands();
    auto rootShaderObject = gDevice->createRootShaderObject(gProgram);
    encoder->setPipelineState(gPipelineState);
    encoder->bindRootShaderObject(rootShaderObject);
    encoder->dispatchCompute(1, 1, 1);
    encoder->endEncoding();
    commandBuffer->close();
    queue->executeCommandBuffer(commandBuffer);
    // TODO: need to copy from the print buffer to a staging buffer...

    ComPtr<ISlangBlob> blob;
    gDevice->readBufferResource(printBuffer, 0, printBufferSize, blob.writeRef());

    gGPUPrinting.processGPUPrintCommands(blob->getBufferPointer(), printBufferSize);

    return SLANG_OK;
}

};

int main()
{
    ExampleProgram app;
    if (SLANG_FAILED(app.execute()))
    {
        return -1;
    }
    return 0;
}
