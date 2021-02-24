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
#include "tools/graphics-app-framework/window.h"
#include "source/core/slang-basic.h"

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
        reportError("%s", (const char*) diagnosticsBlob->getBufferPointer());
    }
}

// The main interesting part of the host application code is where we
// load, compile, inspect, and compose the Slang shader code.
//
Result loadShaderProgram(gfx::IRenderer* renderer, ComPtr<gfx::IShaderProgram>& outShaderProgram)
{
    // We need to obatin a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    // Our example application uses the `gfx` graphics API abstraction layer, which already
    // creates a Slang compilation session for us, so we just grab and use it here.
    ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(renderer->getSlangSession(slangSession.writeRef()));
    
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

    // Given a linked program (one with no unresolved external references,
    // and no not-yet-set specialization parameters), we can request kernel
    // code for the entry points of that program by their indices.
    //
    // Because Slang supports a session with multiple active code generation
    // targets, we also need to specify the index of the target we want
    // code for, but since we have only a single target in this application,
    // there isn't actually a choice.
    //
    int targetIndex = 0;
    //
    // Note: it is possible to get diagnostic messages when generating kernel
    // code, but this is not a common occurence. Most semantic errors in
    // user code will be detected at earlier steps in this compilation flow,
    // but there are certain errors that are currently caught during final
    // code generation (e.g., when using a function that is specific to one
    // target, and then requesting kernel code for another target).
    //
    ComPtr<ISlangBlob> vertexShaderBlob;
    result = linkedProgram->getEntryPointCode(vertexEntryPointIndex, targetIndex, vertexShaderBlob.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);
    ComPtr<ISlangBlob> fragmentShaderBlob;
    result = linkedProgram->getEntryPointCode(fragmentEntryPointIndex, targetIndex, fragmentShaderBlob.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    // Once kernel code has been extracted from Slang, the rest of the logic
    // here is basically the same as in the `hello-world` example.

    char const* vertexCode = (char const*) vertexShaderBlob->getBufferPointer();
    char const* vertexCodeEnd = vertexCode + vertexShaderBlob->getBufferSize();

    char const* fragmentCode = (char const*) fragmentShaderBlob->getBufferPointer();
    char const* fragmentCodeEnd = fragmentCode + fragmentShaderBlob->getBufferSize();

    gfx::IShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Vertex,    vertexCode,     vertexCodeEnd },
        { gfx::StageType::Fragment,  fragmentCode,   fragmentCodeEnd },
    };

    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto shaderProgram = renderer->createProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

int gWindowWidth = 1024;
int gWindowHeight = 768;
const uint32_t kSwapchainImageCount = 2;

gfx::ApplicationContext*    gAppContext;
gfx::Window*                gWindow;
Slang::ComPtr<gfx::IRenderer> gRenderer;
ComPtr<gfx::IBufferResource> gConstantBuffer;
ComPtr<gfx::IPipelineLayout> gPipelineLayout;
ComPtr<gfx::IPipelineState> gPipelineState;
ComPtr<gfx::IDescriptorSet> gDescriptorSet;
ComPtr<gfx::IBufferResource> gVertexBuffer;
ComPtr<gfx::ISwapchain> gSwapchain;
Slang::List<ComPtr<gfx::IFramebuffer>> gFramebuffers;

Result initialize()
{
    WindowDesc windowDesc;
    windowDesc.title = "Slang Shader Toy";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    windowDesc.eventHandler = &_handleEvent;
    windowDesc.userData = this;
    gWindow = createWindow(windowDesc);

    IRenderer::Desc rendererDesc;
    rendererDesc.rendererType = RendererType::DirectX11;
    Result res = gfxCreateRenderer(&rendererDesc, gRenderer.writeRef());
    if(SLANG_FAILED(res)) return res;

    int constantBufferSize = sizeof(Uniforms);

    IBufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(constantBufferSize);
    constantBufferDesc.setDefaults(IResource::Usage::ConstantBuffer);
    constantBufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;

    gConstantBuffer = gRenderer->createBufferResource(
        IResource::Usage::ConstantBuffer,
        constantBufferDesc);
    if(!gConstantBuffer) return SLANG_FAIL;

    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::RG_Float32, offsetof(FullScreenTriangle::Vertex, position) },
    };
    auto inputLayout = gRenderer->createInputLayout(
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));
    if(!inputLayout) return SLANG_FAIL;

    IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(FullScreenTriangle::kVertexCount * sizeof(FullScreenTriangle::Vertex));
    vertexBufferDesc.setDefaults(IResource::Usage::VertexBuffer);
    gVertexBuffer = gRenderer->createBufferResource(
        IResource::Usage::VertexBuffer,
        vertexBufferDesc,
        &FullScreenTriangle::kVertices[0]);
    if(!gVertexBuffer) return SLANG_FAIL;

    ComPtr<IShaderProgram> shaderProgram;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(gRenderer, shaderProgram));

    IDescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        IDescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::UniformBuffer),
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

    auto descriptorSet = gRenderer->createDescriptorSet(descriptorSetLayout, IDescriptorSet::Flag::Transient);
    if(!descriptorSet) return SLANG_FAIL;

    descriptorSet->setConstantBuffer(0, 0, gConstantBuffer);

    gDescriptorSet = descriptorSet;

    // Create swapchain and framebuffers.
    gfx::ISwapchain::Desc swapchainDesc = {};
    swapchainDesc.format = gfx::Format::RGBA_Unorm_UInt8;
    swapchainDesc.width = gWindowWidth;
    swapchainDesc.height = gWindowHeight;
    swapchainDesc.imageCount = kSwapchainImageCount;
    gSwapchain = gRenderer->createSwapchain(
        swapchainDesc, gfx::WindowHandle::FromHwnd(getPlatformWindowHandle(gWindow)));

    IFramebufferLayout::AttachmentLayout renderTargetLayout = {gSwapchain->getDesc().format, 1};
    IFramebufferLayout::AttachmentLayout depthLayout = {gfx::Format::D_Float32, 1};
    IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &renderTargetLayout;
    framebufferLayoutDesc.depthStencil = &depthLayout;
    ComPtr<IFramebufferLayout> framebufferLayout;
    SLANG_RETURN_ON_FAIL(
        gRenderer->createFramebufferLayout(framebufferLayoutDesc, framebufferLayout.writeRef()));

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

        ComPtr<gfx::ITextureResource> depthBufferResource = gRenderer->createTextureResource(
            gfx::IResource::Usage::DepthWrite, depthBufferDesc, nullptr);
        ComPtr<gfx::ITextureResource> colorBuffer;
        gSwapchain->getImage(i, colorBuffer.writeRef());

        gfx::IResourceView::Desc colorBufferViewDesc;
        memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
        colorBufferViewDesc.format = gSwapchain->getDesc().format;
        colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
        ComPtr<gfx::IResourceView> rtv =
            gRenderer->createTextureView(colorBuffer.get(), colorBufferViewDesc);

        gfx::IResourceView::Desc depthBufferViewDesc;
        memset(&depthBufferViewDesc, 0, sizeof(depthBufferViewDesc));
        depthBufferViewDesc.format = gfx::Format::D_Float32;
        depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
        ComPtr<gfx::IResourceView> dsv =
            gRenderer->createTextureView(depthBufferResource.get(), depthBufferViewDesc);

        gfx::IFramebuffer::Desc framebufferDesc;
        framebufferDesc.renderTargetCount = 1;
        framebufferDesc.depthStencilView = dsv.get();
        framebufferDesc.renderTargetViews = rtv.readRef();
        framebufferDesc.layout = framebufferLayout;
        ComPtr<gfx::IFramebuffer> frameBuffer = gRenderer->createFramebuffer(framebufferDesc);
        gFramebuffers.add(frameBuffer);
    }

    // Create pipeline.
    GraphicsPipelineStateDesc desc;
    desc.inputLayout = inputLayout;
    desc.program = shaderProgram;
    desc.framebufferLayout = framebufferLayout;
    desc.pipelineLayout = pipelineLayout;
    auto pipelineState = gRenderer->createGraphicsPipelineState(desc);
    if (!pipelineState)
        return SLANG_FAIL;

    gPipelineState = pipelineState;

    showWindow(gWindow);

    return SLANG_OK;
}

bool wasMouseDown = false;
bool isMouseDown = false;
float lastMouseX = 0.0f;
float lastMouseY = 0.0f;
float clickMouseX = 0.0f;
float clickMouseY = 0.0f;

bool firstTime = true;
uint64_t startTime = 0;

void renderFrame()
{
    gRenderer->beginFrame();
    auto frameIndex = gSwapchain->acquireNextImage();
    gRenderer->setFramebuffer(gFramebuffers[frameIndex]);
    if( firstTime )
    {
        startTime = getCurrentTime();
        firstTime = false;
    }

    gfx::Viewport viewport = {};
    viewport.maxZ = 1.0f;
    viewport.extentX = (float)gWindowWidth;
    viewport.extentY = (float)gWindowHeight;
    gRenderer->setViewportAndScissor(viewport);

    static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
    gRenderer->setClearColor(kClearColor);
    gRenderer->clearFrame();

    if(Uniforms* uniforms = (Uniforms*) gRenderer->map(gConstantBuffer, MapFlavor::WriteDiscard))
    {
        bool isMouseClick = isMouseDown && !wasMouseDown;
        wasMouseDown = isMouseDown;

        if( isMouseClick )
        {
            clickMouseX = lastMouseX;
            clickMouseY = lastMouseY;
        }

        uniforms->iMouse[0] = lastMouseX;
        uniforms->iMouse[1] = lastMouseY;
        uniforms->iMouse[2] = isMouseDown ? clickMouseX : -clickMouseX;
        uniforms->iMouse[3] = isMouseClick ? clickMouseY : -clickMouseY;
        uniforms->iTime = float( double(getCurrentTime() - startTime) / double(getTimerFrequency()) );
        uniforms->iResolution[0] = float(gWindowWidth);
        uniforms->iResolution[1] = float(gWindowHeight);

        gRenderer->unmap(gConstantBuffer);
    }

    gRenderer->setPipelineState(gPipelineState);
    gRenderer->setDescriptorSet(PipelineType::Graphics, gPipelineLayout, 0, gDescriptorSet);

    gRenderer->setVertexBuffer(0, gVertexBuffer, sizeof(FullScreenTriangle::Vertex));
    gRenderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    gRenderer->draw(3);

    gRenderer->makeSwapchainImagePresentable(gSwapchain);

    gRenderer->endFrame();

    gSwapchain->present();
}

void finalize()
{
    gRenderer->waitForGpu();
    destroyWindow(gWindow);
}

void handleEvent(Event const& event)
{
    switch( event.code )
    {
    case EventCode::MouseDown:
        isMouseDown = true;
        lastMouseX = event.u.mouse.x;
        lastMouseY = event.u.mouse.y;
        break;

    case EventCode::MouseMoved:
        lastMouseX = event.u.mouse.x;
        lastMouseY = event.u.mouse.y;
        break;

    case EventCode::MouseUp:
        isMouseDown = false;
        lastMouseX = event.u.mouse.x;
        lastMouseY = event.u.mouse.y;
        break;

    default:
        break;
    }
}

static void _handleEvent(Event const& event)
{
    ShaderToyApp* app = (ShaderToyApp*) getUserData(event.window);
    app->handleEvent(event);
}


};

void innerMain(ApplicationContext* context)
{
    ShaderToyApp app;

    if (SLANG_FAILED(app.initialize()))
    {
        return exitApplication(context, 1);
    }

    while(dispatchEvents(context))
    {
        app.renderFrame();
    }

    app.finalize();
}

GFX_UI_MAIN(innerMain)
