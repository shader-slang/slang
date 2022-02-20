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
// TODO:
// The last 4 types don't actually need __target_intrinsic because they are not used in this slang file, 
// therefore they are by default not emitted (although ideally they should have the label and it should just work).
//
// Instead, they're used in the generated computeMain_wrapper, which forces them to be declared at the same time as
// computeMain_wrapper is emitted.
//
// Something else worth noting is that __target_intrinsic(cpp) would output the struct type as name rather than
// gfx::name, being able to output gfx::name without needing to write it all out would be a nice goal.
//
namespace gfx {
    __target_intrinsic(cpp, "gfx::IDevice") struct IDevice{};
    __target_intrinsic(cpp, "gfx::IBufferResource") struct IBufferResource{};
    __target_intrinsic(cpp, "gfx::ITransientResourceHeap") struct ITransientResourceHeap{};
    __target_intrinsic(cpp, "gfx::IPipelineState") struct IPipelineState{};
    __target_intrinsic(cpp, "gfx::IShaderProgram") struct IShaderProgram{};
    __target_intrinsic(cpp, "gfx::IResourceView") struct IResourceView{};
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
