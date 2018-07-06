// hello.cpp

// This file implements an extremely simple example of loading and
// executing a Slang shader program.
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
#include "slang-graphics/render.h"
#include "slang-graphics/render-d3d11.h"
#include "slang-graphics/window.h"
using namespace slang_graphics;

// We will start with a function that will invoke the Slang compiler
// to generate target-specific code from a shader file, and then
// use that to initialize an API shader program.
//
// Note that `Renderer` and `ShaderProgram` here are types from
// the graphics API abstraction layer, and *not* part of the
// Slang API. This function is representative of code that a user
// might write to integrate Slang into their renderer/engine.
//
ShaderProgram* loadShaderProgram(Renderer* renderer)
{
    // First, we need to create a "session" for interacting with the Slang
    // compiler. This scopes all of our application's interactions
    // with the Slang library. At the moment, creating a session causes
    // Slang to load and validate its standard library, so this is a
    // somewhat heavy-weight operation. When possible, an application
    // should try to re-use the same session across multiple compiles.
    SlangSession* slangSession = spCreateSession(NULL);

    // A compile request represents a single invocation of the compiler,
    // to process some inputs and produce outputs (or errors).
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // We would like to request a single target (output) format: DirectX shader bytecode (DXBC)
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);

    // We will specify the desired "profile" for this one target in terms of the
    // DirectX "shader model" that should be supported.
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_4_0"));

    // A compile request can include one or more "translation units," which more or
    // less amount to individual source files (think `.c` files, not the `.h` files they
    // might include).
    //
    // For this example, our code will all be in the Slang language. The user may
    // also specify HLSL input here, but that currently doesn't affect the compiler's
    // behavior much.
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    // We will load source code for our translation unit from the file `hello.slang`.
    // There are also variations of this API for adding source code from application-provided buffers.
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, "hello.slang");

    // Next we will specify the entry points we'd like to compile.
    // It is often convenient to put more than one entry point in the same file,
    // and the Slang API makes it convenient to use a single run of the compiler
    // to compile all entry points.
    //
    // For each entry point, we need to specify the name of a function, the
    // translation unit in which that function can be found, and the stage
    // that we need to compile for (e.g., vertex, fragment, geometry, ...).
    //
    char const* vertexEntryPointName = "vertexMain";
    char const* fragmentEntryPointName = "fragmentMain";
    int vertexIndex = spAddEntryPoint(slangRequest, translationUnitIndex, vertexEntryPointName, SLANG_STAGE_VERTEX);
    int fragmentIndex = spAddEntryPoint(slangRequest, translationUnitIndex, fragmentEntryPointName, SLANG_STAGE_FRAGMENT);

    // Once all of the input options for the compiler have been specified,
    // we can invoke `spCompile` to run the compiler and see if any errors
    // were detected.
    //
    const SlangResult compileRes = spCompile(slangRequest);

    // Even if there were no errors that forced compilation to fail, the
    // compiler may have produced "diagnostic" output such as warnings.
    // We will go ahead and print that output here.
    //
    if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        reportError("%s", diagnostics);
    }

    // If compilation failed, there is no point in continuing any further.
    if (SLANG_FAILED(compileRes))
    {
        spDestroyCompileRequest(slangRequest);
        spDestroySession(slangSession);
        return nullptr;
    }

    // If compilation was successful, then we will extract the code for
    // our two entry points as "blobs".
    //
    // If you are using a D3D API, then your application may want to
    // take advantage of the fact taht these blobs are binary compatible
    // with the `ID3DBlob`, `ID3D10Blob`, etc. interfaces.

    ISlangBlob* vertexShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, vertexIndex, 0, &vertexShaderBlob);

    ISlangBlob* fragmentShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, fragmentIndex, 0, &fragmentShaderBlob);

    // We extract the begin/end pointers to the output code buffers
    // using operations on the `ISlangBlob` interface.
    char const* vertexCode = (char const*)vertexShaderBlob->getBufferPointer();
    char const* vertexCodeEnd = vertexCode + vertexShaderBlob->getBufferSize();

    char const* fragmentCode = (char const*)fragmentShaderBlob->getBufferPointer();
    char const* fragmentCodeEnd = fragmentCode + fragmentShaderBlob->getBufferSize();

    // Once we have extract the output blobs, it is safe to destroy
    // the compile request and even the session.
    //
    spDestroyCompileRequest(slangRequest);
    spDestroySession(slangSession);

    // Now we use the operations of the example graphics API abstraction
    // layer to load shader code into the underlying API.
    //
    // Reminder: this section does not involve the Slang API at all.
    //

    ShaderProgram::KernelDesc kernelDescs[] =
    {
        { StageType::Vertex,    vertexCode,     vertexCodeEnd },
        { StageType::Fragment,  fragmentCode,   fragmentCodeEnd },
    };

    ShaderProgram::Desc programDesc;
    programDesc.pipelineType = PipelineType::Graphics;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    ShaderProgram* shaderProgram = renderer->createProgram(programDesc);

    // Once we've used the output blobs from the Slang compiler to initialize
    // the API-specific shader program, we can release their memory.
    //
    vertexShaderBlob->release();
    fragmentShaderBlob->release();

    return shaderProgram;
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
static int gWindowWidth = 1024;
static int gWindowHeight = 768;

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
    { { 0,  0, 0.5 },{ 1, 0, 0 } },
    { { 0,  1, 0.5 },{ 0, 0, 1 } },
    { { 1,  0, 0.5 },{ 0, 1, 0 } },
};

// We will define global variables for the various platform and
// graphics API objects that our application needs:
//
// As a reminder, *none* of these are Slang API objects. All
// of them come from the utility library we are using to simplify
// building an example program.
//
ApplicationContext* gAppContext;
Window* gWindow;
Renderer* gRenderer;
BufferResource* gConstantBuffer;
InputLayout* gInputLayout;
BufferResource* gVertexBuffer;
ShaderProgram* gShaderProgram;
BindingState* gBindingState;

SlangResult initialize()
{
    // Create a window for our application to render into.
    WindowDesc windowDesc;
    windowDesc.title = "Hello, World!";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    gWindow = createWindow(windowDesc);

    // Initialize the rendering layer.
    //
    // Note: for now we are hard-coding logic to use the
    // Direct3D11 back-end for the graphics API abstraction.
    // A future version of this example may support multiple
    // platforms/APIs.
    //
    gRenderer = createD3D11Renderer();
    Renderer::Desc rendererDesc;
    rendererDesc.width = gWindowWidth;
    rendererDesc.height = gWindowHeight;
    {
        const SlangResult res = gRenderer->initialize(rendererDesc, getPlatformWindowHandle(gWindow));
        if (SLANG_FAILED(res)) return res;
    }

    // Create a constant buffer for passing the model-view-projection matrix.
    //
    // TODO: A future version of this example will show how to
    // use the Slang reflection API to query the required size
    // for the data in this constant buffer.
    //
    int constantBufferSize = 16 * sizeof(float);

    BufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(constantBufferSize);
    constantBufferDesc.setDefaults(Resource::Usage::ConstantBuffer);
    constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;

    gConstantBuffer = gRenderer->createBufferResource(
        Resource::Usage::ConstantBuffer,
        constantBufferDesc);
    if (!gConstantBuffer) return SLANG_FAIL;

    // Input Assembler (IA)

    // Input Layout

    InputElementDesc inputElements[] = {
        { "POSITION", 0, Format::RGB_Float32, offsetof(Vertex, position) },
        { "COLOR",    0, Format::RGB_Float32, offsetof(Vertex, color) },
    };
    gInputLayout = gRenderer->createInputLayout(
        &inputElements[0],
        2);
    if (!gInputLayout) return SLANG_FAIL;

    // Vertex Buffer

    BufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(kVertexCount * sizeof(Vertex));
    vertexBufferDesc.setDefaults(Resource::Usage::VertexBuffer);

    gVertexBuffer = gRenderer->createBufferResource(
        Resource::Usage::VertexBuffer,
        vertexBufferDesc,
        &kVertexData[0]);
    if (!gVertexBuffer) return SLANG_FAIL;

    // Shaders (VS, PS, ...)

    gShaderProgram = loadShaderProgram(gRenderer);
    if (!gShaderProgram) return SLANG_FAIL;

    // Resource binding state

    BindingState::Desc bindingStateDesc;
    bindingStateDesc.addBufferResource(gConstantBuffer, BindingState::RegisterRange::makeSingle(0));
    gBindingState = gRenderer->createBindingState(bindingStateDesc);

    // Once we've initialized all the graphics API objects,
    // it is time to show our application window and start rendering.

    showWindow(gWindow);

    return SLANG_OK;
}

void renderFrame()
{
    // Clear our framebuffer (color target only)
    //
    static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
    gRenderer->setClearColor(kClearColor);
    gRenderer->clearFrame();

    // We update our constant buffer per-frame, just for the purposes
    // of the example, but we don't actually load different data
    // per-frame (we always use an identity projection).
    //
    if (float* data = (float*)gRenderer->map(gConstantBuffer, MapFlavor::WriteDiscard))
    {
        static const float kIdentity[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 };
        memcpy(data, kIdentity, sizeof(kIdentity));

        gRenderer->unmap(gConstantBuffer);
    }

    // Input Assembler (IA)

    gRenderer->setInputLayout(gInputLayout);
    gRenderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    UInt vertexStride = sizeof(Vertex);
    UInt vertexBufferOffset = 0;
    gRenderer->setVertexBuffers(0, 1, &gVertexBuffer, &vertexStride, &vertexBufferOffset);

    // Vertex Shader (VS)
    // Pixel Shader (PS)

    gRenderer->setShaderProgram(gShaderProgram);
    gRenderer->setBindingState(gBindingState);

    //

    gRenderer->draw(3);

    gRenderer->presentFrame();
}

void finalize()
{
    // TODO: Proper cleanup.
}

// This "inner" main function is used by the platform abstraction
// layer to deal with differences in how an entry point needs
// to be defined for different platforms.
//
void innerMain(ApplicationContext* context)
{
    if (SLANG_FAILED(initialize()))
    {
        return exitApplication(context, 1);
    }

    while (dispatchEvents(context))
    {
        renderFrame();
    }

    finalize();
}

// This macro instantiates an appropriate main function to
// invoke the `innerMain` above.
//
SG_UI_MAIN(innerMain)
