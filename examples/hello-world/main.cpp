// main.cpp

// This file implements an extremely simple example of loading and
// executing a Slang shader program. This is primarily an example
// of how to use Slang as a "drop-in" replacement for an existing
// HLSL compiler like the `D3DCompile` API. More advanced usage
// of advanced Slang language and API features is left to the
// next example.
//
// The comments in the file will attempt to explain concepts as
// they are introduced.
//
// Of course, in order to use the Slang API, we need to include
// its header. We have set up the build options for this project
// so that it is as simple as:
//
#include <slang.h>
//
// Other build setups are possible, and Slang doesn't assume that
// its include directory must be added to your global include
// path.

// For the purposes of keeping the demo code as simple as possible,
// while still retaining some level of portability, our examples
// make use of a small platform and graphics API abstraction layer,
// which is included in the Slang source distribution under the
// `tools/` directory.
//
// Applications can of course use Slang without ever touching this
// abstraction layer, so we will not focus on it when explaining
// examples, except in places where best practices for interacting
// with Slang may depend on an application/engine making certain
// design choices in their abstraction layer.
//
#include "slang-gfx.h"
#include "gfx-util/shader-cursor.h"
#include "tools/platform/window.h"
#include "slang-com-ptr.h"
#include "source/core/slang-basic.h"

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

// The example application will be implemented as a `struct`, so that
// we can scope the resources it allocates without using global variables.
//
struct HelloWorld
{

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

// The main task an application cares about is compiling shader code
// from souce (if needed) and loading it through the chosen graphics API.
//
// In addition, an application may want to receive reflection information
// about the program, which is what a `slang::ProgramLayout` provides.
//
gfx::Result loadShaderProgram(
    gfx::IRenderer*         renderer,
    gfx::IShaderProgram**   outProgram)
{
    // We need to obatin a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    // Our example application uses the `gfx` graphics API abstraction layer, which already
    // creates a Slang compilation session for us, so we just grab and use it here.
    ComPtr<slang::ISession> slangSession;
    slangSession = renderer->getSlangSession();

    // We can now start loading code into the slang session.
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
    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule("shaders", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if(!module)
        return SLANG_FAIL;

    // Loading the `shaders` module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("vertex")] void vertexMain(...)`). Without that information there
    // is no umambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
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
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.slangProgram = linkedProgram;
    SLANG_RETURN_ON_FAIL(renderer->createProgram(programDesc, outProgram));

    return SLANG_OK;
}

//
// The above function shows the core of what is required to use the
// Slang API as a simple compiler (e.g., a drop-in replacement for
// fxc or dxc).
//
// The rest of this file implements an extremely simple rendering application
// that will execute the vertex/fragment shaders loaded with the function
// we have just defined.
//

// We will hard-code the size of our rendering window.
//
int gWindowWidth = 1024;
int gWindowHeight = 768;
const uint32_t kSwapchainImageCount = 2;

// We will define global variables for the various platform and
// graphics API objects that our application needs:
//
// As a reminder, *none* of these are Slang API objects. All
// of them come from the utility library we are using to simplify
// building an example program.
//
RefPtr<platform::Window> gWindow;
Slang::ComPtr<gfx::IRenderer>       gRenderer;

ComPtr<gfx::IPipelineState> gPipelineState;
ComPtr<gfx::IShaderObject> gRootObject;
ComPtr<gfx::ISwapchain> gSwapchain;
List<ComPtr<gfx::IFramebuffer>> gFramebuffers;
ComPtr<gfx::IBufferResource> gVertexBuffer;
ComPtr<gfx::IRenderPassLayout> gRenderPass;
ComPtr<gfx::ICommandQueue> gQueue;

// Now that we've covered the function that actually loads and
// compiles our Slang shade code, we can go through the rest
// of the application code without as much commentary.
//
Slang::Result initialize()
{
    // Create a window for our application to render into.
    //
    platform::WindowDesc windowDesc;
    windowDesc.title = "Hello, World!";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    gWindow = platform::Application::createWindow(windowDesc);
    gWindow->events.mainLoop = [this]() { renderFrame(); };
    // Initialize the rendering layer.
    //
    // Note: for now we are hard-coding logic to use the
    // Direct3D11 back-end for the graphics API abstraction.
    // A future version of this example may support multiple
    // platforms/APIs.
    //
    IRenderer::Desc rendererDesc = {};
    rendererDesc.rendererType = gfx::RendererType::DirectX11;
    gfx::Result res = gfxCreateRenderer(&rendererDesc, gRenderer.writeRef());
    if(SLANG_FAILED(res)) return res;

    ICommandQueue::Desc queueDesc = {};
    queueDesc.type = ICommandQueue::QueueType::Graphics;
    gQueue = gRenderer->createCommandQueue(queueDesc);

    // Now we will create objects needed to configur the "input assembler"
    // (IA) stage of the D3D pipeline.
    //
    // First, we create an input layout:
    //
    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::RGB_Float32, offsetof(Vertex, position) },
        { "COLOR",    0, Format::RGB_Float32, offsetof(Vertex, color) },
    };
    auto inputLayout = gRenderer->createInputLayout(
        &inputElements[0],
        2);
    if(!inputLayout) return SLANG_FAIL;

    // Next we allocate a vertex buffer for our pre-initialized
    // vertex data.
    //
    IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(kVertexCount * sizeof(Vertex));
    vertexBufferDesc.setDefaults(IResource::Usage::VertexBuffer);
    gVertexBuffer = gRenderer->createBufferResource(
        IResource::Usage::VertexBuffer,
        vertexBufferDesc,
        &kVertexData[0]);
    if(!gVertexBuffer) return SLANG_FAIL;

    // Now we will use our `loadShaderProgram` function to load
    // the code from `shaders.slang` into the graphics API.
    //
    ComPtr<IShaderProgram> shaderProgram;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(gRenderer, shaderProgram.writeRef()));

    // In order to bind shader parameters to the pipeline, we need
    // to know how those parameters were assigned to locations/bindings/registers
    // for the target graphics API.
    //
    // The Slang compiler assigns locations to parameters in a deterministic
    // fashion, so it is possible for a programmer to hard-code locations
    // into their application code that will match up with their shaders.
    //
    // Hard-coding of locations can become intractable as an application needs
    // to support more different target platforms and graphics APIs, as well
    // as more shaders with different specialized variants.
    //
    // Rather than rely on hard-coded locations, our examples will make use of
    // reflection information provided by the Slang compiler (see `programLayout`
    // above), and our example graphics API layer will translate that reflection
    // information into a layout for a "root shader object."
    //
    // The root object will store values/bindings for all of the parameters in
    // the `shaderProgram`. At a conceptual level we can think of `rootObject` as
    // representing the "global scope" of the shader program that was loaded;
    // it has entries for each global shader parameter that was declared.
    //
    // Multiple root objects can be created from the same program, and will have
    // separate storage for parameter values.
    //
    // Readers who are familiar with D3D12 or Vulkan might think of this root
    // layout as being similar in spirit to a "root signature" or "pipeline layout."
    //
    ComPtr<IShaderObject> rootObject;
    SLANG_RETURN_ON_FAIL(gRenderer->createRootShaderObject(shaderProgram, rootObject.writeRef()));
    gRootObject = rootObject;

    // Create swapchain and framebuffers.
    gfx::ISwapchain::Desc swapchainDesc = {};
    swapchainDesc.format = gfx::Format::RGBA_Unorm_UInt8;
    swapchainDesc.width = gWindowWidth;
    swapchainDesc.height = gWindowHeight;
    swapchainDesc.imageCount = kSwapchainImageCount;
    swapchainDesc.queue = gQueue;
    gfx::WindowHandle windowHandle;
    memcpy(&windowHandle, &gWindow->getNativeHandle(), sizeof(windowHandle));
    gSwapchain = gRenderer->createSwapchain(swapchainDesc, windowHandle);

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
        gfx::ITextureResource::Desc depthBufferDesc = {};
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

        gfx::IResourceView::Desc colorBufferViewDesc = {};
        colorBufferViewDesc.format = gSwapchain->getDesc().format;
        colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
        ComPtr<gfx::IResourceView> rtv =
            gRenderer->createTextureView(colorBuffer.get(), colorBufferViewDesc);

        gfx::IResourceView::Desc depthBufferViewDesc = {};
        depthBufferViewDesc.format = gfx::Format::D_Float32;
        depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
        ComPtr<gfx::IResourceView> dsv =
            gRenderer->createTextureView(depthBufferResource.get(), depthBufferViewDesc);

        gfx::IFramebuffer::Desc framebufferDesc = {};
        framebufferDesc.renderTargetCount = 1;
        framebufferDesc.depthStencilView = dsv.get();
        framebufferDesc.renderTargetViews = rtv.readRef();
        framebufferDesc.layout = framebufferLayout;
        ComPtr<gfx::IFramebuffer> frameBuffer = gRenderer->createFramebuffer(framebufferDesc);
        gFramebuffers.add(frameBuffer);
    }

    // Following the D3D12/Vulkan style of API, we need a pipeline state object
    // (PSO) to encapsulate the configuration of the overall graphics pipeline.
    //
    GraphicsPipelineStateDesc desc;
    desc.inputLayout = inputLayout;
    desc.program = shaderProgram;
    desc.framebufferLayout = framebufferLayout;
    auto pipelineState = gRenderer->createGraphicsPipelineState(desc);
    if (!pipelineState)
        return SLANG_FAIL;

    gPipelineState = pipelineState;

    gfx::IRenderPassLayout::Desc renderPassDesc = {};
    renderPassDesc.framebufferLayout = framebufferLayout;
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
    gRenderPass = gRenderer->createRenderPassLayout(renderPassDesc);

    return SLANG_OK;
}

// With the initialization out of the way, we can now turn our attention
// to the per-frame rendering logic. As with the initialization, there is
// nothing really Slang-specific here, so the commentary doesn't need
// to be very detailed.
//
void renderFrame()
{
    uint32_t frameBufferIndex = gSwapchain->acquireNextImage();

    ComPtr<ICommandBuffer> commandBuffer = gQueue->createCommandBuffer();
    auto renderEncoder = commandBuffer->encodeRenderCommands(gRenderPass, gFramebuffers[frameBufferIndex]);

    gfx::Viewport viewport = {};
    viewport.maxZ = 1.0f;
    viewport.extentX = (float)gWindowWidth;
    viewport.extentY = (float)gWindowHeight;
    renderEncoder->setViewportAndScissor(viewport);

    // We will update the model-view-projection matrix that is passed
    // into the shader code via the `Uniforms` buffer on a per-frame
    // basis, even though the data that is loaded does not change
    // per-frame (we always use an identity matrix).
    //
    float identityMatrix[] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    gfxGetIdentityProjection(gfxGetProjectionStyle(gRenderer->getRendererType()), identityMatrix);

    //
    // We know that `gRootObject` is a root shader object created
    // from our program, and that it is set up to hold values for
    // all the parameter of that program. In order to actually
    // set values, we need to be able to look up the location
    // of speciic parameter that we want to set.
    //
    // Our example graphics API layer supports this operation
    // with the idea of a *shader cursor* which can be thought
    // of as pointing "into" a particular shader object at
    // some location/offset. This design choice abstracts over
    // the many ways that different platforms and APIs represent
    // the necessary offset information.
    //
    // We construct an initial shader cursor that points at the
    // entire shader program. You can think of this as akin to
    // a diretory path of `/` for the root directory in a file
    // system.
    //
    ShaderCursor rootCursor(gRootObject);
    //
    // Next, we use a convenience overload of `operator[]` to
    // navigate from the root cursor down to the parameter we
    // want to set.
    //
    // The operation `rootCursor["Uniforms"]` looks up the
    // offset/location of the global shader parameter `Uniforms`
    // (which is a uniform/constant buffer), and the subsequent
    // `["modelViewProjection"]` step navigates from there down
    // to the member named `modelViewProjection` in that buffer.
    //
    // Once we have formed a cursor that "points" at the
    // model-view projection matrix, we can set its data directly.
    //
    rootCursor["Uniforms"]["modelViewProjection"].setData(identityMatrix, sizeof(identityMatrix));
    //
    // Some readers might be concerned about the performance o
    // the above operations because of the use of strings. For
    // those readers, here are two things to note:
    //
    // * While these `operator[]` steps do need to perform string
    //   comparisons, they do *not* make copies of the strings or
    //   perform any heap allocation.
    //
    // * There are other overloads of `operator[]` that use the
    //   *index* of a parameter/field instead of its name, and those
    //   operations have fixed/constant overhead and perform no
    //   string comparisons. The indices used are independent of
    //   the target platform and graphics API, and can thus be
    //   hard-coded even in cross-platform code.
    //

    // Now we configure our graphics pipeline state by setting the
    // PSO, binding our root shader object to it (which references
    // the `Uniforms` buffer that will filled in above).
    //
    renderEncoder->setPipelineState(gPipelineState);
    renderEncoder->bindRootShaderObject(gRootObject);

    // We also need to set up a few pieces of fixed-function pipeline
    // state that are not bound by the pipeline state above.
    //
    renderEncoder->setVertexBuffer(0, gVertexBuffer, sizeof(Vertex));
    renderEncoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    // Finally, we are ready to issue a draw call for a single triangle.
    //
    renderEncoder->draw(3);
    renderEncoder->endEncoding();
    commandBuffer->close();
    gQueue->executeCommandBuffer(commandBuffer);

    // With that, we are done drawing for one frame, and ready for the next.
    //
    gSwapchain->present();
}

void finalize()
{
    gQueue->wait();
    gSwapchain = nullptr;
}

};

// This "inner" main function is used by the platform abstraction
// layer to deal with differences in how an entry point needs
// to be defined for different platforms.
//
int innerMain()
{
    // We construct an instance of our example application
    // `struct` type, and then walk through the lifecyle
    // of the application.

    HelloWorld app;

    if (SLANG_FAILED(app.initialize()))
    {
        return -1;
    }

    platform::Application::run(app.gWindow);

    app.finalize();

    return 0;
}

// This macro instantiates an appropriate main function to
// invoke the `innerMain` above.
//
PLATFORM_UI_MAIN(innerMain)
