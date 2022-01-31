// main.cpp

// This example uses the Slang gfx layer to target different APIs and execute
// both CPU and GPU code from a single Slang file (?)
//
#include <slang.h>
#include <slang-com-ptr.h>
using Slang::ComPtr;

#include "slang-gfx.h"
#include "gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"
#include "../../prelude/slang-cpp-types.h"

using namespace gfx;
using namespace Slang;

// Creating global ref pointers to avoid dereferencing values
//
ComPtr<gfx::IDevice> gDevice;
ComPtr<gfx::IShaderProgram> gProgram;
ComPtr<gfx::IBufferResource> gBufferResource;
ComPtr<gfx::IResourceView> gResourceView;
ComPtr<gfx::ITransientResourceHeap> gTransientHeap;
ComPtr<gfx::IPipelineState> gPipelineState;
ComPtr<gfx::ICommandQueue> gQueue;

// Boilerplate types to help the slang-generated file
//
bool executeComputation();

// Many Slang API functions return detailed diagnostic information
// (error messages, warnings, etc.) as a "blob" of data, or return
// a null blob pointer instead if there were no issues.
//
// For convenience, we define a subroutine that will dump the information
// in a diagnostic blob if one is produced, and skip it otherwise.
//
void diagnoseIfNeeded(slang::IBlob *diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        printf("%s", (const char *)diagnosticsBlob->getBufferPointer());
    }
}

gfx::IDevice *createDevice()
{
    ComPtr<gfx::IDevice> device;
    IDevice::Desc deviceDesc = {};
    // Changing device type would happen here. For example:
    //deviceDesc.deviceType = DeviceType::CUDA;
    SLANG_RETURN_NULL_ON_FAIL(gfxCreateDevice(&deviceDesc, gDevice.writeRef()));
    return gDevice;
}

// Loads the shader code defined in `shader.slang` for use by the `gfx` layer.
//
gfx::IShaderProgram *loadShaderProgram(gfx::IDevice *device, char* entryPoint, char* moduleName)
{
    // We need to obtain a compilation session (`slang::ISession`) that will provide
    // a scope to all the compilation and loading of code we do.
    //
    ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_NULL_ON_FAIL(device->getSlangSession(slangSession.writeRef()));

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
    slang::IModule *module = slangSession->loadModule(moduleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return NULL;

    // Look up entry point
    //
    // char const *computeEntryPointName = entryPoint.getBuffer();
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_NULL_ON_FAIL(
        module->findEntryPointByName(entryPoint, computeEntryPoint.writeRef()));

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
    Slang::List<slang::IComponentType *> componentTypes;
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
    SLANG_RETURN_NULL_ON_FAIL(result);

    // At this point, `composedProgram` represents the shader program
    // we want to run, and the compute shader there have been checked.
    // We can create a `gfx::IShaderProgram` object from `composedProgram`
    // so it may be used by the graphics layer.
    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.slangProgram = composedProgram.get();

    gProgram = device->createProgram(programDesc);

    return gProgram;
}

gfx::IBufferResource* createStructuredBuffer(gfx::IDevice* device, FixedArray<float, 4> initialData)
{
    // Create a structured buffer for storing computation data
    //
    const int numberCount = 4;
    int structuredBufferSize = numberCount * sizeof(float);

    IBufferResource::Desc bufferDesc = {};
    bufferDesc.sizeInBytes = numberCount * sizeof(float);
    bufferDesc.format = gfx::Format::Unknown;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.allowedStates = ResourceStateSet(ResourceState::ShaderResource,
                                                ResourceState::UnorderedAccess,
                                                ResourceState::CopyDestination,
                                                ResourceState::CopySource);
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    SlangResult result = device->createBufferResource(bufferDesc,
                                                      (void *)&initialData,
                                                      gBufferResource.writeRef());
    SLANG_RETURN_NULL_ON_FAIL(result);
    return gBufferResource;
}

gfx::IResourceView *createBufferView(
    gfx::IDevice *device,
    gfx::IBufferResource *buffer)
{
    // Create a resource view for the structured buffer
    //
    gfx::IResourceView::Desc viewDesc = {};
    viewDesc.type = gfx::IResourceView::Type::UnorderedAccess;
    viewDesc.format = gfx::Format::Unknown;
    SLANG_RETURN_NULL_ON_FAIL(device->createBufferView(buffer, viewDesc, gResourceView.writeRef()));
    return gResourceView;
}

gfx::ITransientResourceHeap *buildTransientHeap(gfx::IDevice *device)
{
    ITransientResourceHeap::Desc transientHeapDesc = {};
    transientHeapDesc.constantBufferSize = 4096;
    SLANG_RETURN_NULL_ON_FAIL(
        device->createTransientResourceHeap(transientHeapDesc, gTransientHeap.writeRef()));
    return gTransientHeap;
}

gfx::IPipelineState *buildPipelineState(
    gfx::IDevice *device,
    gfx::IShaderProgram *shaderProgram)
{
    gfx::ComputePipelineStateDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram;
    SLANG_RETURN_NULL_ON_FAIL(
        device->createComputePipelineState(pipelineDesc, gPipelineState.writeRef()));
    return gPipelineState;
}

void printInitialValues(FixedArray<float, 4> initialArray, int length)
{
    printf("Before:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%f, ", initialArray[i]);
    }
    printf("\n");
}

void dispatchComputation(
    gfx::IDevice *device,
    gfx::ITransientResourceHeap *transientHeap,
    gfx::IPipelineState *pipelineState,
    gfx::IResourceView *bufferView,
    unsigned int gridDimsX,
    unsigned int gridDimsY,
    unsigned int gridDimsZ)
{
    ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
    gQueue = device->createCommandQueue(queueDesc);

    auto commandBuffer = transientHeap->createCommandBuffer();
    auto encoder = commandBuffer->encodeComputeCommands();

    // First, obtain a root shader object from command encoder to start parameter binding.
    auto rootObject = encoder->bindPipeline(pipelineState);

    gfx::ShaderCursor entryPointCursor(
        rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
    // Bind buffer view to the entry point.
    entryPointCursor.getPath("ioBuffer").setResource(bufferView);

    encoder->dispatchCompute(gridDimsX, gridDimsY, gridDimsZ);
    encoder->endEncoding();
    commandBuffer->close();
    gQueue->executeCommandBuffer(commandBuffer);
    gQueue->waitOnHost();
}

bool printOutputValues(
    gfx::IDevice *device,
    gfx::IBufferResource *buffer,
    int length)
{
    ComPtr<ISlangBlob> resultBlob;
    SLANG_RETURN_FALSE_ON_FAIL(device->readBufferResource(
        buffer, 0, length * sizeof(float), resultBlob.writeRef()));
    auto result = reinterpret_cast<const float *>(resultBlob->getBufferPointer());
    printf("After: \n");
    for (int i = 0; i < length; i++)
    {
        printf("%f, ", result[i]);
    }
    printf("\n");
    return true;
}

RWStructuredBuffer<float> convertBuffer(gfx::IBufferResource* _0) {
    RWStructuredBuffer<float> result;
    result.data = (float*)_0;
    return result;
}

gfx::IBufferResource *unconvertBuffer(RWStructuredBuffer<float> _0)
{
    return (gfx::IBufferResource *)(_0.data);
}

int main()
{
    // We construct an instance of our example application
    // `struct` type, and then walk through the lifecyle
    // of the application.

    if (!(executeComputation()))
    {
        return -1;
    }
}
