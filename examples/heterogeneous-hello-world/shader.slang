// shader.slang

//TEST_INPUT:ubuffer(random(float, 4096, -1.0, 1.0), stride=4):name=ioBuffer
RWStructuredBuffer<float> convertBuffer(Ptr<gfx::BufferResource> x);

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
    struct Device{};
    struct BufferResource{};
    struct ResourceView{};
    struct TransientResourceHeap{};
    struct PipelineState{};
    struct ShaderProgram{};
}

// Forward declarations of cpp functions
//
Ptr<gfx::Device> createDevice();
Ptr<gfx::ShaderProgram> loadShaderProgram(Ptr<gfx::Device> device);
Ptr<gfx::BufferResource> createStructuredBuffer(
    Ptr<gfx::Device> device,
    float[4] initialData);
Ptr<gfx::ResourceView> createBufferView(
    Ptr<gfx::Device> device,
    Ptr<gfx::BufferResource> buffer);
Ptr<gfx::TransientResourceHeap> buildTransientHeap(
    Ptr<gfx::Device> device);
Ptr<gfx::PipelineState> buildPipelineState(
    Ptr<gfx::Device> device,
    Ptr<gfx::ShaderProgram> shaderProgram);
void printInitialValues(float[4] initialArray, int length);
void dispatchComputation(
    Ptr<gfx::Device> device,
    Ptr<gfx::TransientResourceHeap> transientHeap,
    Ptr<gfx::PipelineState> pipelineState,
    Ptr<gfx::ResourceView> bufferView);
bool printOutputValues(
    Ptr<gfx::Device> device,
    Ptr<gfx::BufferResource> buffer,
    int length);

public bool executeComputation() {
    // We will hard-code the size of our initial array.
    //
    float   initialArray[4] = { 3.0f, -20.0f, -6.0f, 8.0f };

    // Declare functions
    let device = createDevice();
    let structuredBuffer = createStructuredBuffer(device, initialArray);
    let bufferView = createBufferView(device, structuredBuffer);
    __GPU_FOREACH(device, uint3(4, 1, 1), LAMBDA(uint3 dispatchThreadID)
        { computeMain(convertBuffer(structuredBuffer), dispatchThreadID) ; });
    printInitialValues(initialArray, 4);
    printOutputValues(device, structuredBuffer, 4);


    return true;
}
