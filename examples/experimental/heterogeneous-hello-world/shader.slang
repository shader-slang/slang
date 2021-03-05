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
    struct ApplicationContext{};
    struct Window{};
    struct Renderer{};
    struct BufferResource{};
    struct PipelineLayout{};
    struct PipelineState{};
    struct DescriptorSetLayout{};
    struct DescriptorSet{};
    struct ShaderProgram{};
}

// Forward declarations of cpp functions
//
Ptr<gfx::ShaderProgram> loadShaderProgram(Ptr<gfx::Renderer> renderer);
Ptr<gfx::Window> createWindow(int gWindowWidth, int gWindowHeight);
Ptr<gfx::Renderer> createRenderer(
    int gWindowWidth,
    int gWindowHeight,
    Ptr<gfx::Window> gWindow);
Ptr<gfx::BufferResource> createStructuredBuffer(Ptr<gfx::Renderer> gRenderer, float[4] initialArray);
void printInitialValues(float[4] initialArray, int length);
void print_output(
    Ptr<gfx::Renderer> gRenderer,
    Ptr<gfx::BufferResource> gStructuredBuffer,
    int length);

public bool executeComputation() {
    // We will hard-code the size of our rendering window and initial array.
    //
    int     windowWidth = 1024;
    int     windowHeight = 768;
    float   initialArray[4] = { 3.0f, -20.0f, -6.0f, 8.0f };

    // Declare functions
    let window = createWindow(windowWidth, windowHeight);
    let renderer = createRenderer(windowWidth, windowHeight, window);
    let structuredBuffer = createStructuredBuffer(renderer, initialArray);
    __GPU_FOREACH(renderer, uint3(4, 1, 1), LAMBDA(uint3 dispatchThreadID)
        { computeMain(convertBuffer(structuredBuffer), dispatchThreadID) ; });
    printInitialValues(initialArray, 4);
    print_output(renderer, structuredBuffer, 4);


    return true;
}
