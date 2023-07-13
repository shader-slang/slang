// main.cpp

#include <slang.h>
#include "slang-gfx.h"
#include "gfx-util/shader-cursor.h"
#include "tools/platform/window.h"
#include "slang-com-ptr.h"
#include "source/core/slang-basic.h"
#include "examples/example-base/example-base.h"

#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"

using namespace gfx;
using namespace Slang;

// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//
struct Vertex
{
    float position[3];
    float color[3];
};

static const int kVertexCount = 3;
static const Vertex kVertexData[kVertexCount] =
{
    { { 0,  0, 0.5 }, { 1, 0, 0 } },
    { { 0,  1, 0.5 }, { 0, 0, 1 } },
    { { 1,  0, 0.5 }, { 0, 1, 0 } },
};

struct AftermathCrashExample : public WindowedAppBase
{
    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob);
    
    gfx::Result loadShaderProgram( gfx::IDevice* device, gfx::IShaderProgram** outProgram);
   
    Slang::Result initialize();

    virtual void renderFrame(int frameBufferIndex) override;
    
    void aftermathCrashDump(const void* data, const uint32_t dataSizeInBytes)
    {
        // NOTE! This method can be called from *any* thread.

    }

    // Create accessors so we don't have to use g prefixed variables.
    gfx::IDevice* getDevice() { return gDevice; }
    gfx::ICommandQueue* getQueue() { return gQueue; }
    gfx::IFramebufferLayout* getFrameBufferLayout() { return gFramebufferLayout; }
    gfx::ISwapchain* getSwapChain() { return gSwapchain; }
    gfx::IRenderPassLayout* getRenderPassLayout() { return gRenderPass; }
    Slang::List<Slang::ComPtr<gfx::IFramebuffer>>& getFrameBuffers() { return gFramebuffers; }
    Slang::List<Slang::ComPtr<gfx::ITransientResourceHeap>>& getTransientHeaps() { return gTransientHeaps; }

    ComPtr<gfx::IPipelineState> m_pipelineState;
    ComPtr<gfx::IBufferResource> m_vertexBuffer;
};

void AftermathCrashExample::diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
    }
}

gfx::Result AftermathCrashExample::loadShaderProgram(
    gfx::IDevice* device,
    gfx::IShaderProgram** outProgram)
{
    ComPtr<slang::ISession> slangSession;
    slangSession = device->getSlangSession();

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule("shaders", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> vertexEntryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("vertexMain", vertexEntryPoint.writeRef()));
    //
    ComPtr<slang::IEntryPoint> fragmentEntryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("fragmentMain", fragmentEntryPoint.writeRef()));

    // At this point we have a few different Slang API objects that represent
    // pieces of our code: `module`, `vertexEntryPoint`, and `fragmentEntryPoint`.
    //
    // A single Slang module could contain many different entry points (e.g.,
    // four vertex entry points, three fragment entry points, and two compute
    // shaders), and before we try to generate output code for our target API
    // we need to identify which entry points we plan to use together.
    //
    // Modules and entry points are both examples of *component types* in the
    // Slang API. The API also provides a way to build a *composite* out of
    // other pieces, and that is what we are going to do with our module
    // and entry points.
    //
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(module);

    // Later on when we go to extract compiled kernel code for our vertex
    // and fragment shaders, we will need to make use of their order within
    // the composition, so we will record the relative ordering of the entry
    // points here as we add them.
    int entryPointCount = 0;
    int vertexEntryPointIndex = entryPointCount++;
    componentTypes.add(vertexEntryPoint);

    int fragmentEntryPointIndex = entryPointCount++;
    componentTypes.add(fragmentEntryPoint);

    // Actually creating the composite component type is a single operation
    // on the Slang session, but the operation could potentially fail if
    // something about the composite was invalid (e.g., you are trying to
    // combine multiple copies of the same module), so we need to deal
    // with the possibility of diagnostic output.
    //
    ComPtr<slang::IComponentType> linkedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        linkedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    // Once we've described the particular composition of entry points
    // that we want to compile, we defer to the graphics API layer
    // to extract compiled kernel code and load it into the API-specific
    // program representation.
    //
    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram;
    SLANG_RETURN_ON_FAIL(device->createProgram(programDesc, outProgram));

    return SLANG_OK;
}

static void GFSDK_AFTERMATH_CALL _dumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData)
{
    reinterpret_cast<AftermathCrashExample*>(pUserData)->aftermathCrashDump(pGpuCrashDump, gpuCrashDumpSize);
}

Slang::Result AftermathCrashExample::initialize()
{
    // As per docs must be called before any device is created

    GFSDK_Aftermath_EnableGpuCrashDumps(
        GFSDK_Aftermath_Version_API,
        GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX | GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
        GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default,
        _dumpCallback, 
        nullptr,
        nullptr,
        nullptr,
        this);

    initializeBase("aftermath-crash-example", 1024, 768);

    auto device = getDevice();

    // We will create objects needed to configur the "input assembler"
    // (IA) stage of the D3D pipeline.
    //
    // First, we create an input layout:
    //
    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position) },
        { "COLOR",    0, Format::R32G32B32_FLOAT, offsetof(Vertex, color) },
    };
    auto inputLayout = gDevice->createInputLayout(
        sizeof(Vertex),
        &inputElements[0],
        2);
    if (!inputLayout) return SLANG_FAIL;

    // Next we allocate a vertex buffer for our pre-initialized
    // vertex data.
    //
    IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.type = IResource::Type::Buffer;
    vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
    vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
    m_vertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
    if (!m_vertexBuffer) return SLANG_FAIL;

    // Now we will use our `loadShaderProgram` function to load
    // the code from `shaders.slang` into the graphics API.
    //
    ComPtr<IShaderProgram> shaderProgram;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(device, shaderProgram.writeRef()));

    // Following the D3D12/Vulkan style of API, we need a pipeline state object
    // (PSO) to encapsulate the configuration of the overall graphics pipeline.
    //
    GraphicsPipelineStateDesc desc;
    desc.inputLayout = inputLayout;
    desc.program = shaderProgram;
    desc.framebufferLayout = getFrameBufferLayout();
    auto pipelineState = device->createGraphicsPipelineState(desc);
    if (!pipelineState)
        return SLANG_FAIL;

    m_pipelineState = pipelineState;

    return SLANG_OK;
}

void AftermathCrashExample::renderFrame(int frameBufferIndex) 
{
    ComPtr<ICommandBuffer> commandBuffer = getTransientHeaps()[frameBufferIndex]->createCommandBuffer();
    auto renderEncoder = commandBuffer->encodeRenderCommands(gRenderPass, getFrameBuffers()[frameBufferIndex]);

    gfx::Viewport viewport = {};
    viewport.maxZ = 1.0f;
    viewport.extentX = (float)windowWidth;
    viewport.extentY = (float)windowHeight;
    renderEncoder->setViewportAndScissor(viewport);

    auto rootObject = renderEncoder->bindPipeline(m_pipelineState);

    auto deviceInfo = getDevice()->getDeviceInfo();

    ShaderCursor rootCursor(rootObject);

    rootCursor["Uniforms"]["modelViewProjection"].setData(
        deviceInfo.identityProjectionMatrix, sizeof(float) * 16);

    // We also need to set up a few pieces of fixed-function pipeline
    // state that are not bound by the pipeline state above.
    //
    renderEncoder->setVertexBuffer(0, m_vertexBuffer);
    renderEncoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    // Finally, we are ready to issue a draw call for a single triangle.
    //
    renderEncoder->draw(3);
    renderEncoder->endEncoding();
    commandBuffer->close();
    getQueue()->executeCommandBuffer(commandBuffer);

    // With that, we are done drawing for one frame, and ready for the next.
    //
    getSwapChain()->present();

    // We only want to present one frame...

    platform::Application::quit();
}

// This macro instantiates an appropriate main function to
// run the application defined above.
PLATFORM_UI_MAIN(innerMain<AftermathCrashExample>)
