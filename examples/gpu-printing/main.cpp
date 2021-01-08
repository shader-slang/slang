// main.cpp

#include <slang.h>

#include <slang-com-ptr.h>
using Slang::ComPtr;

#include "gfx/render.h"
#include "gfx/d3d11/render-d3d11.h"
#include "tools/graphics-app-framework/window.h"
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
        reportError("%s", diagnostics);
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

gfx::ApplicationContext*    gAppContext;
gfx::Window*                gWindow;
ComPtr<gfx::IRenderer>      gRenderer;

ComPtr<slang::ISession> gSlangSession;
ComPtr<slang::IModule>   gSlangModule;
RefPtr<gfx::ShaderProgram>  gProgram;

RefPtr<gfx::PipelineLayout> gPipelineLayout;
RefPtr<gfx::PipelineState>  gPipelineState;
RefPtr<gfx::DescriptorSet>  gDescriptorSet;

Dictionary<int, std::string> gHashedStrings;

GPUPrinting gGPUPrinting;

RefPtr<gfx::ShaderProgram> loadComputeProgram(slang::IModule* slangModule, char const* entryPointName)
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

    gfx::ShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Compute,    code,     codeEnd },
    };

    gfx::ShaderProgram::Desc programDesc;
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto shaderProgram = gRenderer->createProgram(programDesc);

    return shaderProgram;
}

Result execute()
{
    WindowDesc windowDesc;
    windowDesc.title = "GPU Printing";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    gWindow = createWindow(windowDesc);

    createD3D11Renderer(gRenderer.writeRef());
    IRenderer::Desc rendererDesc;
    rendererDesc.width = gWindowWidth;
    rendererDesc.height = gWindowHeight;
    {
        Result res = gRenderer->initialize(rendererDesc, getPlatformWindowHandle(gWindow));
        if(SLANG_FAILED(res)) return res;
    }

    gSlangSession = createSlangSession(gRenderer);
    gSlangModule = compileShaderModuleFromFile(gSlangSession, "kernels.slang");

    gProgram = loadComputeProgram(gSlangModule, "computeMain");
    if(!gProgram) return SLANG_FAIL;

    DescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::StorageBuffer),
    };
    DescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = 1;
    descriptorSetLayoutDesc.slotRanges = &slotRanges[0];
    auto descriptorSetLayout = gRenderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    if(!descriptorSetLayout) return SLANG_FAIL;

    PipelineLayout::DescriptorSetDesc descriptorSets[] =
    {
        PipelineLayout::DescriptorSetDesc( descriptorSetLayout ),
    };
    PipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.renderTargetCount = 1;
    pipelineLayoutDesc.descriptorSetCount = 1;
    pipelineLayoutDesc.descriptorSets = &descriptorSets[0];
    auto pipelineLayout = gRenderer->createPipelineLayout(pipelineLayoutDesc);
    if(!pipelineLayout) return SLANG_FAIL;

    gPipelineLayout = pipelineLayout;

    // Once we have the descriptor set layout, we can allocate
    // and fill in a descriptor set to hold our parameters.
    //
    auto descriptorSet = gRenderer->createDescriptorSet(descriptorSetLayout);
    if(!descriptorSet) return SLANG_FAIL;

//    descriptorSet->setConstantBuffer(0, 0, gConstantBuffer);

    gDescriptorSet = descriptorSet;

    ComputePipelineStateDesc desc;
    desc.pipelineLayout = gPipelineLayout;
    desc.program = gProgram;
    auto pipelineState = gRenderer->createComputePipelineState(desc);
    if(!pipelineState) return SLANG_FAIL;

    gPipelineState = pipelineState;

    size_t printBufferSize = 4 * 1024; // use a small-ish (4KB) buffer for print output

    BufferResource::Desc printBufferDesc;
    printBufferDesc.init(printBufferSize);
    printBufferDesc.elementSize = sizeof(uint32_t);
    printBufferDesc.cpuAccessFlags = Resource::AccessFlag::Read; // | Resource::AccessFlag::Write;
    auto printBuffer = gRenderer->createBufferResource(Resource::Usage::UnorderedAccess, printBufferDesc);

    ResourceView::Desc printBufferViewDesc;
    printBufferViewDesc.type = ResourceView::Type::UnorderedAccess;
    auto printBufferView = gRenderer->createBufferView(printBuffer, printBufferViewDesc);

    // TODO: need to copy a zero into the start of the print buffer!

    gDescriptorSet->setResource(0, 0, printBufferView);
    gRenderer->setDescriptorSet(PipelineType::Compute, gPipelineLayout, 0, gDescriptorSet);

    gRenderer->setPipelineState(PipelineType::Compute, gPipelineState);
    gRenderer->dispatchCompute(1, 1, 1);

    // TODO: need to copy from the print buffer to a staging buffer...

    auto printBufferData = (uint32_t*) gRenderer->map(printBuffer, MapFlavor::HostRead);

    gGPUPrinting.processGPUPrintCommands(printBufferData, printBufferSize);

    return SLANG_OK;
}

};

// This "inner" main function is used by the platform abstraction
// layer to deal with differences in how an entry point needs
// to be defined for different platforms.
//
void innerMain(ApplicationContext* context)
{
    ExampleProgram app;

    if (SLANG_FAILED(app.execute()))
    {
        return exitApplication(context, 1);
    }
}

// This macro instantiates an appropriate main function to
// invoke the `innerMain` above.
//
GFX_CONSOLE_MAIN(innerMain)
