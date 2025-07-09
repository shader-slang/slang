// main.cpp

// This file provides the application code for the `shader-object` example.
//

// This example uses the Slang slang-rhi layer to target different APIs.
// The goal is to demonstrate how the Shader Object model implemented in `slang-rhi` layer
// simplifies shader specialization and parameter binding when using `interface` typed
// shader parameters.
//
#include "slang-com-ptr.h"
#include "slang.h"
using Slang::ComPtr;

#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "slang-rhi.h"

#include <slang-rhi/shader-cursor.h>

using namespace rhi;

static const ExampleResources resourceBase("shader-object");

static TestBase testBase;

// Loads the shader code defined in `shader-object.slang` for use by the `slang-rhi` layer.
//
Result loadShaderProgram(
    IDevice* device,
    ComPtr<IShaderProgram>& outShaderProgram,
    slang::ProgramLayout*& slangReflection)
{
    // We need to obtain a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    // Our example application uses the `slang-rhi` graphics API abstraction layer, which already
    // creates a Slang compilation session for us, so we just grab and use it here.
    ComPtr<slang::ISession> slangSession;
    slangSession = device->getSlangSession();

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
    Slang::String path = resourceBase.resolveResource("shader-object.slang");
    slang::IModule* module = slangSession->loadModule(path.getBuffer(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
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
    char const* computeEntryPointName = "computeMain";
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
    if (testBase.isTestMode())
    {
        testBase.printEntrypointHashes(1, 1, composedProgram);
    }

    slangReflection = composedProgram->getLayout();

    // At this point, `composedProgram` represents the shader program
    // we want to run, and the compute shader there have been checked.
    // We can create a `IShaderProgram` object from `composedProgram`
    // so it may be used by the graphics layer.
    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createShaderProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

// Main body of the example.
int exampleMain(int argc, char** argv)
{
    testBase.parseOption(argc, argv);

    // Creates a `slang-rhi` renderer, which provides the main interface for
    // interacting with the graphics API.
    Slang::ComPtr<IDevice> device;
    DeviceDesc deviceDesc = {};
    device = getRHI()->createDevice(deviceDesc);
    if (!device)
        return SLANG_FAIL;

    // Now we can load the shader code.
    // A `IShaderProgram` object for use in the `slang-rhi` layer.
    ComPtr<IShaderProgram> shaderProgram;
    // A composed `IComponentType` that gives us reflection info on the shader code.
    slang::ProgramLayout* slangReflection;
    SLANG_RETURN_ON_FAIL(loadShaderProgram(device, shaderProgram, slangReflection));

    // Create a pipeline state with the loaded shader.
    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    pipelineState = device->createComputePipeline(pipelineDesc);
    if (!pipelineState)
        return SLANG_FAIL;

    // Create and initiate our input/output buffer.
    const int numberCount = 4;
    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    numbersBuffer = device->createBuffer(bufferDesc, (void*)initialData);
    if (!numbersBuffer)
        return SLANG_FAIL;

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);

        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();

        // Now comes the interesting part: binding the shader parameter for the
        // compute kernel that we about to launch. We would like to construct
        // a shader object that represents a `f(x)=x+1` transformation and apply
        // it to the numbers in `numbersBuffer`.

        // First, obtain a root shader object from command encoder to start parameter binding.
        auto rootObject = encoder->bindPipeline(pipelineState);

        // Next, we create a shader object that represents the transformer we want to use.
        // To do so, we first need to lookup for the `AddTransformer` type defined in the shader
        // code.
        slang::TypeReflection* addTransformerType =
            slangReflection->findTypeByName("AddTransformer");

        // Now we can use this type to create a shader object that can be bound to the root object.
        ComPtr<IShaderObject> transformer;
        transformer =
            device->createShaderObject(addTransformerType, ShaderObjectContainerType::None);
        if (!transformer)
            return SLANG_FAIL;

        // Set the `c` field of the `AddTransformer`.
        float c = 1.0f;
        ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

        // We can set parameters directly with `rootObject`, but that requires us to use
        // the Slang reflection API to obtain the proper offsets into the root object for each
        // parameter. We implemented these logic in the `ShaderCursor` helper class, which
        // simplifies the user code to find shader parameters. Here we demonstrate how to set
        // parameters with `ShaderCursor`.
        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(numbersBuffer);

        // Bind the previously created transformer object to root object.
        entryPointCursor.getPath("transformer").setObject(transformer);

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }
    // Read back the results.
    ComPtr<ISlangBlob> resultBlob;
    SLANG_RETURN_ON_FAIL(
        device->readBuffer(numbersBuffer, 0, numberCount * sizeof(float), resultBlob.writeRef()));
    auto result = reinterpret_cast<const float*>(resultBlob->getBufferPointer());
    for (int i = 0; i < numberCount; i++)
        printf("%f\n", result[i]);

    return SLANG_OK;
}
