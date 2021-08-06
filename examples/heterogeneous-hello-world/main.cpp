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
struct gfx_Device_0;
struct gfx_BufferResource_0;
struct gfx_ShaderProgram_0;
struct gfx_ResourceView_0;
struct gfx_TransientResourceHeap_0;
struct gfx_PipelineState_0;
bool executeComputation_0();

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

gfx::IDevice* createDevice()
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
gfx::IShaderProgram* loadShaderProgram(gfx::IDevice *device)
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
    slang::IModule *module = slangSession->loadModule("shader", diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return NULL;

    // Look up entry point (hardcoded for now)
    //
    char const *computeEntryPointName = "computeMain";
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_NULL_ON_FAIL(
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
    programDesc.pipelineType = gfx::PipelineType::Compute;
    programDesc.slangProgram = composedProgram.get();

    gProgram = device->createProgram(programDesc);

    return gProgram;
}

gfx::IBufferResource* createStructuredBuffer(
    gfx::IDevice *device,
    float *initialData)
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
    bufferDesc.cpuAccessFlags = AccessFlag::Write | AccessFlag::Read;

    SlangResult result = device->createBufferResource(bufferDesc,
                                                      (void *)initialData,
                                                      gBufferResource.writeRef());
    SLANG_RETURN_NULL_ON_FAIL(result);
    return gBufferResource;
}

gfx::IResourceView* createBufferView(
    gfx::IDevice* device,
    gfx::IBufferResource* buffer)
{
    // Create a resource view for the structured buffer
    //
    gfx::IResourceView::Desc viewDesc = {};
    viewDesc.type = gfx::IResourceView::Type::UnorderedAccess;
    viewDesc.format = gfx::Format::Unknown;
    SLANG_RETURN_NULL_ON_FAIL(device->createBufferView(buffer, viewDesc, gResourceView.writeRef()));
    return gResourceView;
}

gfx::ITransientResourceHeap* buildTransientHeap(gfx::IDevice *device)
{
    ITransientResourceHeap::Desc transientHeapDesc = {};
    transientHeapDesc.constantBufferSize = 4096;
    SLANG_RETURN_NULL_ON_FAIL(
        device->createTransientResourceHeap(transientHeapDesc, gTransientHeap.writeRef()));
    return gTransientHeap;
}

gfx::IPipelineState* buildPipelineState(
    gfx::IDevice *device,
    gfx::IShaderProgram* shaderProgram)
{
    gfx::ComputePipelineStateDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram;
    SLANG_RETURN_NULL_ON_FAIL(
        device->createComputePipelineState(pipelineDesc, gPipelineState.writeRef()));
    return gPipelineState;
}

void printInitialValues(float *initialArray, int length)
{
    printf("Before:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%f, ", initialArray[i]);
    }
    printf("\n");
}

void dispatchComputation(
    gfx::IDevice* device,
    gfx::ITransientResourceHeap* transientHeap,
    gfx::IPipelineState* pipelineState,
    gfx::IResourceView* bufferView,
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
    gQueue->wait();
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

// Boilerplate functions to help the slang-generated file and types

gfx_Device_0* createDevice_0()
{
     return (gfx_Device_0*)createDevice();
}

gfx_BufferResource_0* createStructuredBuffer_0(gfx_Device_0* _0, FixedArray<float, 4> _1)
{
    return (gfx_BufferResource_0*)createStructuredBuffer((gfx::IDevice*)_0, (float*)&_1);
}

gfx_ShaderProgram_0* loadShaderProgram_0(gfx_Device_0* _0)
{
    return (gfx_ShaderProgram_0*)loadShaderProgram((gfx::IDevice*)_0);
}

gfx_ResourceView_0* createBufferView_0(gfx_Device_0* _0, gfx_BufferResource_0* _1)
{
    return (gfx_ResourceView_0*)createBufferView((gfx::IDevice*)_0, (gfx::IBufferResource*)_1);
}

gfx_TransientResourceHeap_0* buildTransientHeap_0(gfx_Device_0* _0)
{
    return (gfx_TransientResourceHeap_0*)buildTransientHeap((gfx::IDevice*)_0);
}

gfx_PipelineState_0* buildPipelineState_0(gfx_Device_0* _0, gfx_ShaderProgram_0* _1)
{
    return (gfx_PipelineState_0*)buildPipelineState((gfx::IDevice*)_0, (gfx::IShaderProgram*)_1);
}

void printInitialValues_0(FixedArray<float, 4> _0, int32_t _1)
{
    printInitialValues((float*)&_0, _1);
}

void dispatchComputation_0(gfx_Device_0* _0, gfx_TransientResourceHeap_0* _1, gfx_PipelineState_0* _2, gfx_ResourceView_0* _3, unsigned int gridDimsX, unsigned int gridDimsY, unsigned int gridDimsZ)
{
    dispatchComputation(
        (gfx::IDevice*)_0,
        (gfx::ITransientResourceHeap*)_1,
        (gfx::IPipelineState*)_2,
        (gfx::IResourceView*)_3,
        gridDimsX,
        gridDimsY,
        gridDimsZ);
}

RWStructuredBuffer<float> convertBuffer_0(gfx_BufferResource_0* _0) {
    RWStructuredBuffer<float> result;
    result.data = (float*)_0;
    return result;
}

gfx_BufferResource_0* unconvertBuffer_0(RWStructuredBuffer<float> _0) {
    return (gfx_BufferResource_0*)(_0.data);
}

bool printOutputValues_0(gfx_Device_0* _0, gfx_BufferResource_0* _1, int32_t _2)
{
    return printOutputValues((gfx::IDevice*)_0, (gfx::IBufferResource*)_1, _2);
}

int main()
{
    // We construct an instance of our example application
    // `struct` type, and then walk through the lifecyle
    // of the application.

    if (!(executeComputation_0()))
    {
        return -1;
    }
}
