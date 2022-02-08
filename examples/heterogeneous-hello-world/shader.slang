// shader.slang

//TEST_INPUT:ubuffer(random(float, 4096, -1.0, 1.0), stride=4):name=ioBuffer
__unmangled __exportDirectly RWStructuredBuffer<float> convertBuffer(Ptr<gfx::IBufferResource> x);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uniform RWStructuredBuffer<float> ioBuffer, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    float i = ioBuffer[tid];
    float o = i < 0.5 ? (i + i) : sqrt(i);

    ioBuffer[tid] = o;
}



// Forward declarations of gfx types
//
namespace gfx {
    __externLib __exportDirectly struct IDevice{};
    __externLib __exportDirectly struct IBufferResource{};
    __exportDirectly struct ITransientResourceHeap{};
    __exportDirectly struct IPipelineState{};
    __exportDirectly struct IShaderProgram{};
    __exportDirectly struct IResourceView{};
}

// Forward declarations of cpp functions
//
__unmangled __exportDirectly Ptr<gfx::IDevice> createDevice();
__unmangled __exportDirectly Ptr<gfx::IShaderProgram> loadShaderProgram(
    Ptr<gfx::IDevice> device, 
    String entryPoint, 
    String module);
__unmangled __exportDirectly Ptr<gfx::IBufferResource> createStructuredBuffer(
    Ptr<gfx::IDevice> device,
    float[4] initialData);
__unmangled __exportDirectly Ptr<gfx::IResourceView> createBufferView(
    Ptr<gfx::IDevice> device,
    Ptr<gfx::IBufferResource> buffer);
__unmangled __exportDirectly Ptr<gfx::ITransientResourceHeap> buildTransientHeap(
    Ptr<gfx::IDevice> device);
__unmangled __exportDirectly Ptr<gfx::IPipelineState> buildPipelineState(
    Ptr<gfx::IDevice> device,
    Ptr<gfx::IShaderProgram> shaderProgram);
__unmangled __exportDirectly void printInitialValues(float[4] initialArray, int length);
__unmangled __exportDirectly void dispatchComputation(
    Ptr<gfx::IDevice> device,
    Ptr<gfx::ITransientResourceHeap> transientHeap,
    Ptr<gfx::IPipelineState> pipelineState,
    Ptr<gfx::IResourceView> bufferView);
__unmangled __exportDirectly bool printOutputValues(
    Ptr<gfx::IDevice> device,
    Ptr<gfx::IBufferResource> buffer,
    int length);

public __unmangled __exportDirectly bool executeComputation() {
    // We will hard-code the size of our initial array.
    //
    float   initialArray[4] = { 3.0f, -20.0f, -6.0f, 8.0f };

    // Declare functions
    let device = createDevice();
    let structuredBuffer = createStructuredBuffer(device, initialArray);
    // let bufferView = createBufferView(device, structuredBuffer);
    __GPU_FOREACH(device, uint3(4, 1, 1), LAMBDA(uint3 dispatchThreadID)
        { computeMain(convertBuffer(structuredBuffer), dispatchThreadID) ; });
    printInitialValues(initialArray, 4);
    printOutputValues(device, structuredBuffer, 4);


    return true;
}
