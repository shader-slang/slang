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
#include "gfx/render.h"
#include "gfx/d3d11/render-d3d11.h"
#include "gfx/window.h"
using namespace gfx;

// We will start with a function that will invoke the Slang compiler
// to generate target-specific code from a shader file, and then
// use that to initialize an API shader program.
//
// Note that `Renderer` and `ShaderProgram` here are types from
// the graphics API abstraction layer, and *not* part of the
// Slang API. This function is representative of code that a user
// might write to integrate Slang into their renderer/engine.
//
RefPtr<gfx::ShaderProgram> loadShaderProgram(gfx::Renderer* renderer)
{
    // First, we need to create a "session" for interacting with the Slang
    // compiler. This scopes all of our application's interactions
    // with the Slang library. At the moment, creating a session causes
    // Slang to load and validate its standard library, so this is a
    // somewhat heavy-weight operation. When possible, an application
    // should try to re-use the same session across multiple compiles.
    //
    SlangSession* slangSession = spCreateSession(NULL);

    // A compile request represents a single invocation of the compiler,
    // to process some inputs and produce outputs (or errors).
    //
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // We would like to request a single target (output) format: DirectX shader bytecode (DXBC)
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);

    // We will specify the desired "profile" for this one target in terms of the
    // DirectX "shader model" that should be supported.
    //
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_4_0"));

    // A compile request can include one or more "translation units," which more or
    // less amount to individual source files (think `.c` files, not the `.h` files they
    // might include).
    //
    // For this example, our code will all be in the Slang language. The user may
    // also specify HLSL input here, but that currently doesn't affect the compiler's
    // behavior much.
    //
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    // We will load source code for our translation unit from the file `shaders.slang`.
    // There are also variations of this API for adding source code from application-provided buffers.
    //
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, "shader.slang");

    // Next we will specify the entry points we'd like to compile.
    // It is often convenient to put more than one entry point in the same file,
    // and the Slang API makes it convenient to use a single run of the compiler
    // to compile all entry points.
    //
    // For each entry point, we need to specify the name of a function, the
    // translation unit in which that function can be found, and the stage
    // that we need to compile for (e.g., vertex, fragment, geometry, ...).
    //
    char const* computeEntryPointName = "computeMain";
    int computeIndex = spAddEntryPoint(slangRequest, translationUnitIndex, computeEntryPointName,   SLANG_STAGE_COMPUTE);

    // Once all of the input options for the compiler have been specified,
    // we can invoke `spCompile` to run the compiler and see if any errors
    // were detected.
    //
    const SlangResult compileRes = spCompile(slangRequest);

    // Even if there were no errors that forced compilation to fail, the
    // compiler may have produced "diagnostic" output such as warnings.
    // We will go ahead and print that output here.
    //
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        reportError("%s", diagnostics);
    }

    // If compilation failed, there is no point in continuing any further.
    if(SLANG_FAILED(compileRes))
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
    //

    ISlangBlob* computeShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, computeIndex, 0, &computeShaderBlob);

    // We extract the begin/end pointers to the output code buffers
    // using operations on the `ISlangBlob` interface.
    //
    char const* computeCode = (char const*) computeShaderBlob->getBufferPointer();
    char const* computeCodeEnd = computeCode + computeShaderBlob->getBufferSize();

    // Once we have extracted the output blobs, it is safe to destroy
    // the compile request and even the session.
    //
    spDestroyCompileRequest(slangRequest);
    spDestroySession(slangSession);

    // Now we use the operations of the example graphics API abstraction
    // layer to load shader code into the underlying API.
    //
    // Reminder: this section does not involve the Slang API at all.
    //

    gfx::ShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Compute,    computeCode,     computeCodeEnd },
    };

    gfx::ShaderProgram::Desc programDesc;
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto shaderProgram = renderer->createProgram(programDesc);

    // Once we've used the output blobs from the Slang compiler to initialize
    // the API-specific shader program, we can release their memory.
    //
    computeShaderBlob->release();

    return shaderProgram;
}

// Now that we've covered the function that actually loads and
// compiles our Slang shade code, we can go through the rest
// of the application code without as much commentary.
//
Result computeMain()
{
    // We will hard-code the size of our rendering window and initial array.
    //
    int     gWindowWidth = 1024;
    int     gWindowHeight = 768;
    float   initialArray[4] = { 3.0f, -20.0f, -6.0f, 8.0f };

    // We will define global variables for the various platform and
    // graphics API objects that our application needs:
    //
    // As a reminder, *none* of these are Slang API objects. All
    // of them come from the utility library we are using to simplify
    // building an example program.
    //
    gfx::ApplicationContext*    gAppContext;
    gfx::Window*                gWindow;
    RefPtr<gfx::Renderer>       gRenderer;

    RefPtr<gfx::BufferResource> gUnorderedAccess;
    RefPtr<gfx::BufferResource> gReadBuffer;

    RefPtr<gfx::PipelineLayout> gPipelineLayout;
    RefPtr<gfx::PipelineState>  gPipelineState;
    RefPtr<gfx::DescriptorSet>  gDescriptorSet;

    // Create a window for our application to render into.
    //
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
        Result res = gRenderer->initialize(rendererDesc, getPlatformWindowHandle(gWindow));
        if(SLANG_FAILED(res)) return res;
    }

    // Create a structured buffer for passing the compute data
    //
    int structuredBufferSize = 4 * sizeof(float);

    BufferResource::Desc structuredBufferDesc;
    structuredBufferDesc.init(structuredBufferSize);
    structuredBufferDesc.setDefaults(Resource::Usage::UnorderedAccess);
    structuredBufferDesc.elementSize = 4;
    structuredBufferDesc.cpuAccessFlags = Resource::AccessFlag::Read;

    gUnorderedAccess = gRenderer->createBufferResource(
        Resource::Usage::UnorderedAccess,
        structuredBufferDesc,
        initialArray);
    if(!gUnorderedAccess) return SLANG_FAIL;

    // Now we will use our `loadShaderProgram` function to load
    // the code from `shader.slang` into the graphics API.
    //
    RefPtr<ShaderProgram> shaderProgram = loadShaderProgram(gRenderer);
    if(!shaderProgram) return SLANG_FAIL;

    // Our example graphics API usess a "modern" D3D12/Vulkan style
    // of resource binding, so now we will dive into describing and
    // allocating "descriptor sets."
    //
    // First, we need to construct a descriptor set *layout*.
    DescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::StorageBuffer),
    };
    DescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = 1;
    descriptorSetLayoutDesc.slotRanges = &slotRanges[0];
    auto descriptorSetLayout = gRenderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    if(!descriptorSetLayout) return SLANG_FAIL;

    // Next we will allocate a pipeline layout, which specifies
    // that we will render with only a single descriptor set bound.
    //

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

    // Once we have the bufferResource created, we can fill in
    // a descriptor set for creating a structured buffer
    //
    ResourceView::Desc resourceViewDesc;
    resourceViewDesc.type = ResourceView::Type::UnorderedAccess;
    auto resourceView = gRenderer->createBufferView(gUnorderedAccess, resourceViewDesc);
    descriptorSet->setResource(0, 0, resourceView);

    gDescriptorSet = descriptorSet;

    // Following the D3D12/Vulkan style of API, we need a pipeline state object
    // (PSO) to encapsulate the configuration of the overall graphics pipeline.
    //
    ComputePipelineStateDesc desc;
    desc.pipelineLayout = gPipelineLayout;
    desc.program = shaderProgram;
    auto pipelineState = gRenderer->createComputePipelineState(desc);
    if(!pipelineState) return SLANG_FAIL;

    gPipelineState = pipelineState;

    // Once we've initialized all the graphics API objects,
    // it is time to show our application window and start rendering.
    //
    //showWindow(gWindow);

    // Now we configure our graphics pipeline state by setting the
    // PSO, binding our descriptor set (which references the
    // constant buffer that we wrote to above), and setting
    // some additional bits of state, before drawing our triangle.
    //

    // Print out the values before the computation
    printf("Before:\n");
    for (float v : initialArray)
    {
        printf("%f, ", v);
    }
    printf("\n");

    gRenderer->setPipelineState(PipelineType::Compute, gPipelineState);
    gRenderer->setDescriptorSet(PipelineType::Compute, gPipelineLayout, 0, gDescriptorSet);

    gRenderer->dispatchCompute(4, 1, 1);

    if(float* outputData = (float*) gRenderer->map(gUnorderedAccess, MapFlavor::HostRead))
    {
        // Print out the values the the kernel produced
        printf("After: \n");
        for (int i = 0; i < 4; i++)
        {
            printf("%f, ", outputData[i]);
        }
        printf("\n");

        gRenderer->unmap(gUnorderedAccess);
    }

    return SLANG_OK;
}

// This "inner" main function is used by the platform abstraction
// layer to deal with differences in how an entry point needs
// to be defined for different platforms.
//
void innerMain(ApplicationContext* context)
{
    // We construct an instance of our example application
    // `struct` type, and then walk through the lifecyle
    // of the application.

    if (SLANG_FAILED(computeMain()))
    {
        return exitApplication(context, 1);
    }
}

// This macro instantiates an appropriate main function to
// invoke the `innerMain` above.
//
GFX_CONSOLE_MAIN(innerMain)
