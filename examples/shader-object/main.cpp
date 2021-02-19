// main.cpp

// This file provides the application code for the `shader-object` example.
//

// This example uses the Slang gfx layer to target different APIs.
// The goal is to demonstrate how the Shader Object model implemented in `gfx` layer
// simplifies shader specialization and parameter binding when using `interface` typed
// shader parameters.
//
#include <slang.h>
#include <slang-com-ptr.h>
using Slang::ComPtr;

#include "slang-gfx.h"
#include "gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

// Helper function for print out diagnostic messages output by Slang compiler.
void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
    }
}

// Loads the shader code defined in `shader-object.slang` for use by the `gfx` layer.
//
Result loadShaderProgram(
    gfx::IRenderer* renderer,
    ComPtr<gfx::IShaderProgram>& outShaderProgram,
    slang::ProgramLayout*& slangReflection)
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
    // Note: The only interesting wrinkle here is that our file is named `shader-object` with
    // a hyphen in it, so the name is not directly usable as an identifier in Slang code.
    // Instead, when trying to import this module in the context of Slang code, a user
    // needs to replace the hyphens with underscores:
    //
    //      import shader_object;
    //
    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule("shader-object", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if(!module)
        return SLANG_FAIL;

    // Loading the `shader-object` module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("vertex")] void vertexMain(...)`). Without that information there
    // is no umambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
    char const* computeEntryPointName    = "computeMain";
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module->findEntryPointByName(computeEntryPointName, computeEntryPoint.writeRef()));
  
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
    componentTypes.add(computeEntryPoint);

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
    slangReflection = composedProgram->getLayout();

    // At this point, `composedProgram` represents the shader program
    // we want to run, and the compute shader there have been checked.
    // We can create a `gfx::IShaderProgram` object from `composedProgram`
    // so it may be used by the graphics layer.
    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.slangProgram = composedProgram.get();

    auto shaderProgram = renderer->createProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

// Main body of the example.
int main()
{
    // Creates a `gfx` renderer, which provides the main interface for
    // interacting with the graphics API.
    Slang::ComPtr<gfx::IRenderer> renderer;
    IRenderer::Desc rendererDesc = {};
    rendererDesc.rendererType = RendererType::CUDA;
    SLANG_RETURN_ON_FAIL(gfxCreateRenderer(&rendererDesc, nullptr, renderer.writeRef()));

    // Now we can load the shader code.
    // A `gfx::IShaderProgram` object for use in the `gfx` layer.
    ComPtr<gfx::IShaderProgram> shaderProgram;
    // A composed `IComponentType` that gives us reflection info on the shader code.
    slang::ProgramLayout* slangReflection;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(renderer, shaderProgram, slangReflection));

    // Create a pipelien state with the loaded shader.
    gfx::ComputePipelineStateDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<gfx::IPipelineState> pipelineState;
    SLANG_RETURN_ON_FAIL(
        renderer->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

    // Create and initiate our input/output buffer.
    const int numberCount = 4;
    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.sizeInBytes = numberCount * sizeof(float);
    bufferDesc.format = gfx::Format::Unknown;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.bindFlags = gfx::IResource::BindFlag::NonPixelShaderResource |
                           gfx::IResource::BindFlag::UnorderedAccess;
    bufferDesc.cpuAccessFlags = IResource::AccessFlag::Write | IResource::AccessFlag::Read;

    ComPtr<gfx::IBufferResource> numbersBuffer;
    SLANG_RETURN_ON_FAIL(renderer->createBufferResource(
        gfx::IResource::Usage::UnorderedAccess,
        bufferDesc,
        (void*)initialData,
        numbersBuffer.writeRef()));

    // Create a resource view for the buffer.
    ComPtr<gfx::IResourceView> bufferView;
    gfx::IResourceView::Desc viewDesc = {};
    viewDesc.type = gfx::IResourceView::Type::UnorderedAccess;
    viewDesc.format = gfx::Format::Unknown;
    SLANG_RETURN_ON_FAIL(renderer->createBufferView(numbersBuffer, viewDesc, bufferView.writeRef()));

    // Now comes the interesting part: binding the shader parameter for the
    // compute kernel that we about to launch. We would like to construct
    // a shader object that represents a `f(x)=x+1` transformation and apply
    // it to the numbers in `numbersBuffer`.
    // To start, we create a root shader object that represents the root level
    // scope of the shader parameters.
    ComPtr<gfx::IShaderObject> rootObject;
    SLANG_RETURN_ON_FAIL(renderer->createRootShaderObject(shaderProgram, rootObject.writeRef()));
    // We can set parameters directly with `rootObject`, but that requires us to use
    // the Slang reflection API to obtain the proper offsets into the root object for each parameter.
    // We implemented these logic in the `ShaderCursor` helper class, which simplifies the user
    // code to find shader parameters. Here we demonstrate how to set parameters with `ShaderCursor`.
    gfx::ShaderCursor entryPointCursor(rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
    // Bind buffer view to the entry point.
    entryPointCursor.getPath("buffer").setResource(bufferView);

    // Next, we create a shader object that represents the transformer we want to use.
    // To do so, we first need to lookup for the `AddTransformer` type defined in the shader code.
    slang::TypeReflection* addTransformerType = slangReflection->findTypeByName("AddTransformer");

    // Now we can use this type to create a shader object that can be bound to the root object.
    ComPtr<gfx::IShaderObject> transformer;
    SLANG_RETURN_ON_FAIL(
        renderer->createShaderObject(addTransformerType, transformer.writeRef()));
    // Set the `c` field of the `AddTransformer`.
    float c = 1.0f;
    gfx::ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

    // Now the transformer object is ready, we can bind it to root object.
    entryPointCursor.getPath("transformer").setObject(transformer);

    // We have set up all required parameters in entry-point object, now it is time
    // to bind the pipeline and root object and launch the kernel.
    renderer->setPipelineState(pipelineState);
    SLANG_RETURN_ON_FAIL(renderer->bindRootShaderObject(gfx::PipelineType::Compute, rootObject));
    renderer->dispatchCompute(1, 1, 1);

    // Read back the results.
    renderer->waitForGpu();
    float* result = (float*)renderer->map(numbersBuffer, gfx::MapFlavor::HostRead);
    for (int i = 0; i < numberCount; i++)
        printf("%f\n", result[i]);
    renderer->unmap(numbersBuffer);

    return SLANG_OK;
}
