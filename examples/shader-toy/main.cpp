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
#include "gfx/render.h"
#include "gfx/d3d11/render-d3d11.h"
#include "tools/graphics-app-framework/window.h"
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
Result loadShaderProgram(gfx::IRenderer* renderer, RefPtr<gfx::ShaderProgram>& outShaderProgram)
{
    // The first step in interacting with the Slang API is to create a "global session,"
    // which represents an instance of the Slang API loaded from the library.
    //
    ComPtr<slang::IGlobalSession> slangGlobalSession;
    SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, slangGlobalSession.writeRef()));

    // Next, we need to create a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    // In an application like this, which doesn't make use of preprocessor-based specialization,
    // we can create a single session and use it for the duration of the application.
    // One important service the session provides is re-use of modules that have already
    // been compiled, so that if two Slang files `import` the same module, the compiler
    // will only load and check that module once.
    //
    // When creating a session we need to tell it what code generation targets we may
    // want code generated for. It is valid to have zero or more targets, but many
    // applications will only want one, corresponding to the graphics API they plan to use.
    // This application is currently hard-coded to use D3D11, so we set up for compilation
    // to DX bytecode.
    //
    // Note: the `TargetDesc` can also be used to set things like optimization settings
    // for each target, but this application doesn't care to set any of that stuff.
    //
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_DXBC;
    targetDesc.profile = spFindProfile(slangGlobalSession, "sm_4_0");

    // The session can be set up with a few other options, notably:
    //
    // * Any search paths that should be used when resolving `import` or `#include` directives.
    //
    // * Any preprocessor macros to pre-define when reading in files.
    //
    // This application doesn't plan to make heavy use of the preprocessor, and all its
    // shader files are in the same directory, so we just use the default options (which
    // will lead to the only search path being the current working directory).
    //
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(slangGlobalSession->createSession(sessionDesc, slangSession.writeRef()));

    // Once the session has been created, we can start loading code into it.
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
    List<slang::IComponentType*> componentTypes;
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
    List<slang::SpecializationArg> specializationArgs;

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

    gfx::ShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Vertex,    vertexCode,     vertexCodeEnd },
        { gfx::StageType::Fragment,  fragmentCode,   fragmentCodeEnd },
    };

    gfx::ShaderProgram::Desc programDesc;
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto shaderProgram = renderer->createProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

int gWindowWidth = 1024;
int gWindowHeight = 768;

gfx::ApplicationContext*    gAppContext;
gfx::Window*                gWindow;
Slang::ComPtr<gfx::IRenderer> gRenderer;
RefPtr<gfx::BufferResource> gConstantBuffer;

RefPtr<gfx::PipelineLayout> gPipelineLayout;
RefPtr<gfx::PipelineState>  gPipelineState;
RefPtr<gfx::DescriptorSet>  gDescriptorSet;

RefPtr<gfx::BufferResource> gVertexBuffer;

Result initialize()
{
    WindowDesc windowDesc;
    windowDesc.title = "Slang Shader Toy";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    windowDesc.eventHandler = &_handleEvent;
    windowDesc.userData = this;
    gWindow = createWindow(windowDesc);

    createD3D11Renderer(gRenderer.writeRef());
    IRenderer::Desc rendererDesc;
    rendererDesc.width = gWindowWidth;
    rendererDesc.height = gWindowHeight;
    {
        Result res = gRenderer->initialize(rendererDesc, getPlatformWindowHandle(gWindow));
        if(SLANG_FAILED(res)) return res;
    }

    int constantBufferSize = sizeof(Uniforms);

    BufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(constantBufferSize);
    constantBufferDesc.setDefaults(Resource::Usage::ConstantBuffer);
    constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;

    gConstantBuffer = gRenderer->createBufferResource(
        Resource::Usage::ConstantBuffer,
        constantBufferDesc);
    if(!gConstantBuffer) return SLANG_FAIL;

    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::RG_Float32, offsetof(FullScreenTriangle::Vertex, position) },
    };
    auto inputLayout = gRenderer->createInputLayout(
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));
    if(!inputLayout) return SLANG_FAIL;

    BufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(FullScreenTriangle::kVertexCount * sizeof(FullScreenTriangle::Vertex));
    vertexBufferDesc.setDefaults(Resource::Usage::VertexBuffer);
    gVertexBuffer = gRenderer->createBufferResource(
        Resource::Usage::VertexBuffer,
        vertexBufferDesc,
        &FullScreenTriangle::kVertices[0]);
    if(!gVertexBuffer) return SLANG_FAIL;

    RefPtr<ShaderProgram> shaderProgram;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(gRenderer, shaderProgram));

    DescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::UniformBuffer),
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

    auto descriptorSet = gRenderer->createDescriptorSet(descriptorSetLayout);
    if(!descriptorSet) return SLANG_FAIL;

    descriptorSet->setConstantBuffer(0, 0, gConstantBuffer);

    gDescriptorSet = descriptorSet;

    GraphicsPipelineStateDesc desc;
    desc.pipelineLayout = gPipelineLayout;
    desc.inputLayout = inputLayout;
    desc.program = shaderProgram;
    desc.renderTargetCount = 1;
    auto pipelineState = gRenderer->createGraphicsPipelineState(desc);
    if(!pipelineState) return SLANG_FAIL;

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
    if( firstTime )
    {
        startTime = getCurrentTime();
        firstTime = false;
    }

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

    gRenderer->setPipelineState(PipelineType::Graphics, gPipelineState);
    gRenderer->setDescriptorSet(PipelineType::Graphics, gPipelineLayout, 0, gDescriptorSet);

    gRenderer->setVertexBuffer(0, gVertexBuffer, sizeof(FullScreenTriangle::Vertex));
    gRenderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    gRenderer->draw(3);

    gRenderer->presentFrame();
}

void finalize()
{
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
