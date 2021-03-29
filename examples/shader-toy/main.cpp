// main.cpp

// This file provides the application code for the `shader-toy` example.
//
// Much of the logic here is identical to the simpler `hello-world` example,
// so we will not spend time commenting those parts that are identical or
// nearly identical. Readers who want detailed comments on a simpler example
// using Slang should look there.

// This example uses the Slang C/C++ API, alonmg with its optional type
// for managing COM-style reference-counted pointers.
//
#include <slang.h>
#include <slang-com-ptr.h>
using Slang::ComPtr;

// This example uses a graphics API abstraction layer that is implemented inside
// the Slang codebase for use in our sample programs and test cases. Use of
// this layer is *not* required or assumed when using the Slang language,
// compiler, and API.
//
#include "slang-gfx.h"
#include "tools/gfx-util/shader-cursor.h"
#include "tools/platform/window.h"
#include "tools/platform/performance-counter.h"
#include "source/core/slang-basic.h"

#include <chrono>

using namespace gfx;

// In order to display a shader toy effect using rasterization-based shader
// execution we need to render a full-screen triangle. We will define a
// small helper type that defines the data for such a triangle.
//
struct FullScreenTriangle
{
    struct Vertex
    {
        float position[2];
    };

    enum { kVertexCount = 3 };

    static const Vertex kVertices[kVertexCount];
};
const FullScreenTriangle::Vertex FullScreenTriangle::kVertices[FullScreenTriangle::kVertexCount] =
{
    { { -1,  -1 } },
    { { -1,   3 } },
    { {  3,  -1 } },
};

// The application itself will be encapsulated in a C++ `struct` type
// so that it can easily scope its state without use of global variables.
//
struct ShaderToyApp
{

// The uniform data used by the shader is defined here as a simple
// POD ("plain old data") type.
//
// Note: This type must match the declaration of `ShaderToyUniforms`
// in the file `shader-toy.slang`.
//
// An application could instead use a shared header file to define
// this type, or use Slang's reflection capabilities to allocate
// and set parameters at runtime. For this simple example we did
// the expedient thing of having distinct Slang and C++ declarations.
//
struct Uniforms
{
    float iMouse[4];
    float iResolution[2];
    float iTime;
};

// Many Slang API functions return detailed diagnostic information
// (error messages, warnings, etc.) as a "blob" of data, or return
// a null blob pointer instead if there were no issues.
//
// For convenience, we define a subroutine that will dump the information
// in a diagnostic blob if one is produced, and skip it otherwise.
//
void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if( diagnosticsBlob != nullptr )
    {
        printf("%s", (const char*) diagnosticsBlob->getBufferPointer());
    }
}

// The main interesting part of the host application code is where we
// load, compile, inspect, and compose the Slang shader code.
//
Result loadShaderProgram(gfx::IDevice* device, ComPtr<gfx::IShaderProgram>& outShaderProgram)
{
    // We need to obatin a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    // Our example application uses the `gfx` graphics API abstraction layer, which already
    // creates a Slang compilation session for us, so we just grab and use it here.
    ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    
    // Once the session has been obtained, we can start loading code into it.
    //
    // The simplest way to load code is by calling `loadModule` with the name of a Slang
    // module. A call to `loadModule("MyStuff")` will behave more or less as if you
    // wrote:
    //
    //      import MyStuff;
    //
    // In a Slang shader file. The compiler will use its search paths to try to locate
    // `MyModule.slang`, then compile and load that file. If a matching module had
    // already been loaded previously, that would be used directly.
    //
    // Note: The only interesting wrinkle here is that our file is named `shader-toy` with
    // a hyphen in it, so the name is not directly usable as an identifier in Slang code.
    // Instead, when trying to import this module in the context of Slang code, a user
    // needs to replace the hyphens with underscores:
    //
    //      import shader_toy;
    //
    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule("shader-toy", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if(!module)
        return SLANG_FAIL;

    // Loading the `shader-toy` module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("vertex")] void vertexMain(...)`). Without that information there
    // is no umambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
    char const* vertexEntryPointName    = "vertexMain";
    char const* fragmentEntryPointName  = "fragmentMain";
    //
    ComPtr<slang::IEntryPoint> vertexEntryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName(vertexEntryPointName, vertexEntryPoint.writeRef()));
    //
    ComPtr<slang::IEntryPoint> fragmentEntryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName(fragmentEntryPointName, fragmentEntryPoint.writeRef()));

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
    ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    // At this point, `composedProgram` represents the shader program
    // we want to run, and the vertex and fragment shader there have
    // been checked.
    //
    // We could use the Slang reflection API on `composedProgram` at this
    // point to query things like the locations and offsets of the
    // various uniform parameters, textures, etc.
    //
    // What *cannot* be done yet at this point is actually generating
    // kernel code, because `composedProgram` includes a generic type
    // parameter as part of the `fragmentMain` entry point:
    //
    //      void fragmentMain<T : IShaderToyImageShader>(...)
    //
    // Our next task is to load code for a type we'd like to plug in
    // for `T` there.
    //
    // Because Slang supports modular programming, there is no requirement
    // that a type we want to plug in for `T` has to come from the
    // same module, and to demonstrate that we will load a different
    // module to provide the effect type we will plug in.
    //
    const char* effectModuleName = "example-effect";
    const char* effectTypeName = "ExampleEffect";
    slang::IModule* effectModule = slangSession->loadModule(effectModuleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if(!module)
        return SLANG_FAIL;

    // Once we've loaded the code module that defines out effect type,
    // we can look it up by name using the reflection information on
    // the module.
    //
    // Note: A future version of the Slang API will support enumerating
    // the types declared in a module so that we do not have to hard-code
    // the name here.
    //
    auto effectType = effectModule->getLayout()->findTypeByName(effectTypeName);

    // Now that we have the `effectType` we want to plug in to our generic
    // shader, we need to specialize the shader to that type.
    //
    // Because a shader program could have zero or more specialization parameters,
    // we need to build up an array of specialization arguments.
    //
    Slang::List<slang::SpecializationArg> specializationArgs;

    {
        // In our case, we only have a single specialization argument we plan
        // to use, and it is a type argument.
        //
        slang::SpecializationArg effectTypeArg;
        effectTypeArg.kind = slang::SpecializationArg::Kind::Type;
        effectTypeArg.type = effectType;
        specializationArgs.add(effectTypeArg);
    }

    // Specialization of a component type is a single Slang API call, but
    // we need to deal with the possibility of diagnostic output on failure.
    // For example, if we tried to specialize the shader program to a
    // type like `int` that doesn't support the `IShaderToyImageShader` interface,
    // this is the step where we'd get an error message saying so.
    //
    ComPtr<slang::IComponentType> specializedProgram;
    result = composedProgram->specialize(
        specializationArgs.getBuffer(),
        specializationArgs.getCount(),
        specializedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    // At this point we have a specialized shader program that represents our
    // intention to run the `vertexMain` and `fragmentMain` entry points,
    // specialized to the `ExampleEffect` type we loaded.
    //
    // We can now *link* the program, which ensures that all of the code that
    // it transitively depends on has been pulled together into a single
    // component type.
    //
    ComPtr<slang::IComponentType> linkedProgram;
    result = specializedProgram->link(
        linkedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.slangProgram = linkedProgram.get();
    auto shaderProgram = device->createProgram(programDesc);
    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

int gWindowWidth = 1024;
int gWindowHeight = 768;
static const uint32_t kSwapchainImageCount = 2;

Slang::RefPtr<platform::Window> gWindow;
Slang::ComPtr<gfx::IDevice> gDevice;
ComPtr<IShaderProgram> gShaderProgram;
ComPtr<gfx::IShaderObject> gRootObject[kSwapchainImageCount];
ComPtr<gfx::IFramebufferLayout> gFramebufferLayout;
ComPtr<gfx::IPipelineState> gPipelineState;
ComPtr<gfx::IBufferResource> gVertexBuffer;
ComPtr<gfx::ISwapchain> gSwapchain;
Slang::List<ComPtr<gfx::IFramebuffer>> gFramebuffers;
ComPtr<gfx::IRenderPassLayout> gRenderPass;
ComPtr<gfx::ICommandQueue> gQueue;

Result initialize()
{
    platform::WindowDesc windowDesc;
    const char* title = "Slang Shader Toy";
    windowDesc.title = title;
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    gWindow = platform::Application::createWindow(windowDesc);
    gWindow->events.mainLoop = [this]() { renderFrame(); };
    gWindow->events.mouseMove = [this](const platform::MouseEventArgs& e) { handleEvent(e); };
    gWindow->events.mouseUp = [this](const platform::MouseEventArgs& e) { handleEvent(e); };
    gWindow->events.mouseDown = [this](const platform::MouseEventArgs& e) { handleEvent(e); };
    gWindow->events.sizeChanged = Slang::Action<>(this, &ShaderToyApp::windowSizeChanged);

    IDevice::Desc deviceDesc;
    Result res = gfxCreateDevice(&deviceDesc, gDevice.writeRef());
    if(SLANG_FAILED(res)) return res;

    auto deviceInfo = gDevice->getDeviceInfo();
    Slang::StringBuilder titleSb;
    titleSb << title << " (" << deviceInfo.apiName << ": " << deviceInfo.adapterName << ")";
    gWindow->setText(titleSb.getBuffer());

    ICommandQueue::Desc queueDesc = {};
    queueDesc.type = ICommandQueue::QueueType::Graphics;
    gQueue = gDevice->createCommandQueue(queueDesc);

    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::RG_Float32, offsetof(FullScreenTriangle::Vertex, position) },
    };
    auto inputLayout = gDevice->createInputLayout(
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));
    if(!inputLayout) return SLANG_FAIL;

    IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(FullScreenTriangle::kVertexCount * sizeof(FullScreenTriangle::Vertex));
    vertexBufferDesc.setDefaults(IResource::Usage::VertexBuffer);
    gVertexBuffer = gDevice->createBufferResource(
        IResource::Usage::VertexBuffer,
        vertexBufferDesc,
        &FullScreenTriangle::kVertices[0]);
    if(!gVertexBuffer) return SLANG_FAIL;

    SLANG_RETURN_ON_FAIL(loadShaderProgram(gDevice, gShaderProgram));

    // Create swapchain and framebuffers.
    gfx::ISwapchain::Desc swapchainDesc = {};
    swapchainDesc.format = gfx::Format::RGBA_Unorm_UInt8;
    swapchainDesc.width = gWindowWidth;
    swapchainDesc.height = gWindowHeight;
    swapchainDesc.imageCount = kSwapchainImageCount;
    swapchainDesc.queue = gQueue;
    gSwapchain = gDevice->createSwapchain(swapchainDesc, gWindow->getNativeHandle().convert<gfx::WindowHandle>());

    IFramebufferLayout::AttachmentLayout renderTargetLayout = {gSwapchain->getDesc().format, 1};
    IFramebufferLayout::AttachmentLayout depthLayout = {gfx::Format::D_Float32, 1};
    IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &renderTargetLayout;
    framebufferLayoutDesc.depthStencil = &depthLayout;
    SLANG_RETURN_ON_FAIL(
        gDevice->createFramebufferLayout(framebufferLayoutDesc, gFramebufferLayout.writeRef()));

    createSwapchainFramebuffers();

    // Create pipeline.
    GraphicsPipelineStateDesc desc;
    desc.inputLayout = inputLayout;
    desc.program = gShaderProgram;
    desc.framebufferLayout = gFramebufferLayout;
    auto pipelineState = gDevice->createGraphicsPipelineState(desc);
    if (!pipelineState)
        return SLANG_FAIL;

    gPipelineState = pipelineState;

    // Create render pass.
    gfx::IRenderPassLayout::Desc renderPassDesc = {};
    renderPassDesc.framebufferLayout = gFramebufferLayout;
    renderPassDesc.renderTargetCount = 1;
    IRenderPassLayout::AttachmentAccessDesc renderTargetAccess = {};
    IRenderPassLayout::AttachmentAccessDesc depthStencilAccess = {};
    renderTargetAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    renderTargetAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
    renderTargetAccess.initialState = ResourceState::Undefined;
    renderTargetAccess.finalState = ResourceState::Present;
    depthStencilAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    depthStencilAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
    depthStencilAccess.initialState = ResourceState::Undefined;
    depthStencilAccess.finalState = ResourceState::DepthWrite;
    renderPassDesc.renderTargetAccess = &renderTargetAccess;
    renderPassDesc.depthStencilAccess = &depthStencilAccess;
    gRenderPass = gDevice->createRenderPassLayout(renderPassDesc);
    return SLANG_OK;
}

bool wasMouseDown = false;
bool isMouseDown = false;
float lastMouseX = 0.0f;
float lastMouseY = 0.0f;
float clickMouseX = 0.0f;
float clickMouseY = 0.0f;

bool firstTime = true;
platform::TimePoint startTime;

void renderFrame()
{
    auto frameIndex = gSwapchain->acquireNextImage();
    if (frameIndex == -1)
        return;

    auto commandBuffer = gQueue->createCommandBuffer();
    if( firstTime )
    {
        startTime = platform::PerformanceCounter::now();
        firstTime = false;
    }

    // Update uniform buffer.

    Uniforms uniforms = {};
    {
        bool isMouseClick = isMouseDown && !wasMouseDown;
        wasMouseDown = isMouseDown;

        if( isMouseClick )
        {
            clickMouseX = lastMouseX;
            clickMouseY = lastMouseY;
        }

        uniforms.iMouse[0] = lastMouseX;
        uniforms.iMouse[1] = lastMouseY;
        uniforms.iMouse[2] = isMouseDown ? clickMouseX : -clickMouseX;
        uniforms.iMouse[3] = isMouseClick ? clickMouseY : -clickMouseY;
        uniforms.iTime = platform::PerformanceCounter::getElapsedTimeInSeconds(startTime);
        uniforms.iResolution[0] = float(gWindowWidth);
        uniforms.iResolution[1] = float(gWindowHeight);

    }
    gRootObject[frameIndex] = gDevice->createRootShaderObject(gShaderProgram);
    auto constantBuffer = gRootObject[frameIndex]->getObject(ShaderOffset());
    constantBuffer->setData(ShaderOffset(), &uniforms, sizeof(uniforms));

    // Encode render commands.
    auto encoder = commandBuffer->encodeRenderCommands(gRenderPass, gFramebuffers[frameIndex]);

    gfx::Viewport viewport = {};
    viewport.maxZ = 1.0f;
    viewport.extentX = (float)gWindowWidth;
    viewport.extentY = (float)gWindowHeight;
    encoder->setViewportAndScissor(viewport);
    encoder->setPipelineState(gPipelineState);
    encoder->bindRootShaderObject(gRootObject[frameIndex]);
    encoder->setVertexBuffer(0, gVertexBuffer, sizeof(FullScreenTriangle::Vertex));
    encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);
    encoder->draw(3);
    encoder->endEncoding();
    commandBuffer->close();

    gQueue->executeCommandBuffer(commandBuffer);
    gSwapchain->present();
}

void finalize()
{
    gQueue->wait();
}

void handleEvent(const platform::MouseEventArgs& event)
{
    isMouseDown = ((int)event.buttons & (int)platform::ButtonState::Enum::LeftButton) != 0;
    lastMouseX = (float)event.x;
    lastMouseY = (float)event.y;
}

void createSwapchainFramebuffers()
{
    gFramebuffers.clear();
    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
        gfx::ITextureResource::Desc depthBufferDesc;
        depthBufferDesc.setDefaults(gfx::IResource::Usage::DepthWrite);
        depthBufferDesc.init2D(
            gfx::IResource::Type::Texture2D,
            gfx::Format::D_Float32,
            gSwapchain->getDesc().width,
            gSwapchain->getDesc().height,
            0);

        ComPtr<gfx::ITextureResource> depthBufferResource = gDevice->createTextureResource(
            gfx::IResource::Usage::DepthWrite, depthBufferDesc, nullptr);
        ComPtr<gfx::ITextureResource> colorBuffer;
        gSwapchain->getImage(i, colorBuffer.writeRef());

        gfx::IResourceView::Desc colorBufferViewDesc;
        memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
        colorBufferViewDesc.format = gSwapchain->getDesc().format;
        colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
        ComPtr<gfx::IResourceView> rtv =
            gDevice->createTextureView(colorBuffer.get(), colorBufferViewDesc);

        gfx::IResourceView::Desc depthBufferViewDesc;
        memset(&depthBufferViewDesc, 0, sizeof(depthBufferViewDesc));
        depthBufferViewDesc.format = gfx::Format::D_Float32;
        depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
        ComPtr<gfx::IResourceView> dsv =
            gDevice->createTextureView(depthBufferResource.get(), depthBufferViewDesc);

        gfx::IFramebuffer::Desc framebufferDesc;
        framebufferDesc.renderTargetCount = 1;
        framebufferDesc.depthStencilView = dsv.get();
        framebufferDesc.renderTargetViews = rtv.readRef();
        framebufferDesc.layout = gFramebufferLayout;
        ComPtr<gfx::IFramebuffer> frameBuffer = gDevice->createFramebuffer(framebufferDesc);
        gFramebuffers.add(frameBuffer);
    }
}

void windowSizeChanged()
{
    // Wait for the GPU to finish.
    gQueue->wait();

    auto clientRect = gWindow->getClientRect();
    if (clientRect.width > 0 && clientRect.height > 0)
    {
        // Free all framebuffers before resizing swapchain.
        gFramebuffers = decltype(gFramebuffers)();

        // Resize swapchain.
        if (gSwapchain->resize(clientRect.width, clientRect.height) == SLANG_OK)
        {
            // Recreate framebuffers for each swapchain back buffer image.
            createSwapchainFramebuffers();
            gWindowWidth = clientRect.width;
            gWindowHeight = clientRect.height;
        }
    }
}

};

int innerMain()
{
    ShaderToyApp app;

    if (SLANG_FAILED(app.initialize()))
    {
        return -1;
    }

    platform::Application::run(app.gWindow);

    app.finalize();

    return 0;
}

PLATFORM_UI_MAIN(innerMain)
