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

ComPtr<slang::ISession> createSlangSession(gfx::IRenderer* renderer)
{
    ComPtr<slang::IGlobalSession> slangGlobalSession;
    slangGlobalSession.attach(spCreateSession(NULL));

    slang::TargetDesc targetDesc;
    targetDesc.format = SLANG_DXBC;
    targetDesc.profile = spFindProfile(slangGlobalSession, "sm_5_0");

    slang::SessionDesc sessionDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> slangSession;
    slangGlobalSession->createSession(sessionDesc, slangSession.writeRef());

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

ComPtr<gfx::IRenderer>      gRenderer;

ComPtr<slang::ISession> gSlangSession;
ComPtr<slang::IModule>   gSlangModule;
ComPtr<gfx::IShaderProgram> gProgram;

ComPtr<gfx::IPipelineLayout> gPipelineLayout;
ComPtr<gfx::IPipelineState> gPipelineState;
ComPtr<gfx::IDescriptorSet> gDescriptorSet;

Slang::Dictionary<int, std::string> gHashedStrings;

GPUPrinting gGPUPrinting;

ComPtr<gfx::IShaderProgram> loadComputeProgram(slang::IModule* slangModule, char const* entryPointName)
{
    ComPtr<slang::IEntryPoint> entryPoint;
    slangModule->findEntryPointByName(entryPointName, entryPoint.writeRef());

    ComPtr<slang::IComponentType> linkedProgram;
    entryPoint->link(linkedProgram.writeRef());

    gGPUPrinting.loadStrings(linkedProgram->getLayout());

    ComPtr<ISlangBlob> codeBlob;
    linkedProgram->getEntryPointCode(0, 0, codeBlob.writeRef());

    char const* code = (char const*) codeBlob->getBufferPointer();
    char const* codeEnd = code + codeBlob->getBufferSize();

    gfx::IShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Compute,    code,     codeEnd },
    };

    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto shaderProgram = gRenderer->createProgram(programDesc);

    return shaderProgram;
}

Result execute()
{
    IRenderer::Desc rendererDesc;
    rendererDesc.rendererType = gfx::RendererType::DirectX11;
    Result res = gfxCreateRenderer(&rendererDesc, gRenderer.writeRef());
    if(SLANG_FAILED(res)) return res;

    gSlangSession = createSlangSession(gRenderer);
    gSlangModule = compileShaderModuleFromFile(gSlangSession, "kernels.slang");

    gProgram = loadComputeProgram(gSlangModule, "computeMain");
    if(!gProgram) return SLANG_FAIL;

    IDescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        IDescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::StorageBuffer),
    };
    IDescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = 1;
    descriptorSetLayoutDesc.slotRanges = &slotRanges[0];
    auto descriptorSetLayout = gRenderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    if(!descriptorSetLayout) return SLANG_FAIL;

    IPipelineLayout::DescriptorSetDesc descriptorSets[] =
    {
        IPipelineLayout::DescriptorSetDesc( descriptorSetLayout ),
    };
    IPipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.renderTargetCount = 1;
    pipelineLayoutDesc.descriptorSetCount = 1;
    pipelineLayoutDesc.descriptorSets = &descriptorSets[0];
    auto pipelineLayout = gRenderer->createPipelineLayout(pipelineLayoutDesc);
    if(!pipelineLayout) return SLANG_FAIL;

    gPipelineLayout = pipelineLayout;

    // Once we have the descriptor set layout, we can allocate
    // and fill in a descriptor set to hold our parameters.
    //
    auto descriptorSet = gRenderer->createDescriptorSet(descriptorSetLayout, IDescriptorSet::Flag::Transient);
    if(!descriptorSet) return SLANG_FAIL;

    gDescriptorSet = descriptorSet;

    ComputePipelineStateDesc desc;
    desc.pipelineLayout = gPipelineLayout;
    desc.program = gProgram;
    auto pipelineState = gRenderer->createComputePipelineState(desc);
    if(!pipelineState) return SLANG_FAIL;

    gPipelineState = pipelineState;

    size_t printBufferSize = 4 * 1024; // use a small-ish (4KB) buffer for print output

    IBufferResource::Desc printBufferDesc;
    printBufferDesc.init(printBufferSize);
    printBufferDesc.elementSize = sizeof(uint32_t);
    printBufferDesc.cpuAccessFlags = IResource::AccessFlag::Read; // | Resource::AccessFlag::Write;
    auto printBuffer = gRenderer->createBufferResource(IResource::Usage::UnorderedAccess, printBufferDesc);

    IResourceView::Desc printBufferViewDesc;
    printBufferViewDesc.type = IResourceView::Type::UnorderedAccess;
    auto printBufferView = gRenderer->createBufferView(printBuffer, printBufferViewDesc);

    ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
    auto queue = gRenderer->createCommandQueue(queueDesc);
    auto commandBuffer = queue->createCommandBuffer();
    auto encoder = commandBuffer->encodeComputeCommands();
    // TODO: need to copy a zero into the start of the print buffer!

    gDescriptorSet->setResource(0, 0, printBufferView);
    encoder->setDescriptorSet(gPipelineLayout, 0, gDescriptorSet);

    encoder->setPipelineState(gPipelineState);
    encoder->dispatchCompute(1, 1, 1);
    encoder->endEncoding();
    commandBuffer->close();
    queue->executeCommandBuffer(commandBuffer);
    // TODO: need to copy from the print buffer to a staging buffer...

    ComPtr<ISlangBlob> blob;
    gRenderer->readBufferResource(printBuffer, 0, printBufferSize, blob.writeRef());

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
