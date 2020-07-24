// shader.slang

//TEST_INPUT:ubuffer(random(float, 4096, -1.0, 1.0), stride=4):name=ioBuffer
RWStructuredBuffer<float> ioBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
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
Ptr<gfx::DescriptorSetLayout> buildDescriptorSetLayout(Ptr<gfx::Renderer> gRenderer);
Ptr<gfx::PipelineLayout> buildPipeline(Ptr<gfx::Renderer> gRenderer, Ptr<gfx::DescriptorSetLayout> descriptorSetLayout);
Ptr<gfx::DescriptorSet> buildDescriptorSet(
    Ptr<gfx::Renderer> gRenderer,
    Ptr<gfx::DescriptorSetLayout> descriptorSetLayout,
    Ptr<gfx::BufferResource> gStructuredBuffer);
Ptr<gfx::PipelineState> buildPipelineState(
    Ptr<gfx::ShaderProgram> shaderProgram,
    Ptr<gfx::Renderer> gRenderer,
    Ptr<gfx::PipelineLayout> gPipelineLayout);
void printInitialValues(float[4] initialArray, int length);
void dispatchComputation(
    Ptr<gfx::Renderer> gRenderer,
    Ptr<gfx::PipelineState> gPipelineState,
    Ptr<gfx::PipelineLayout> gPipelineLayout,
    Ptr<gfx::DescriptorSet> gDescriptorSet);
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
    let shaderProgram = loadShaderProgram(renderer);
    let descriptorSetLayout = buildDescriptorSetLayout(renderer);
    let pipelineLayout = buildPipeline(renderer, descriptorSetLayout);
    let descriptorSet = buildDescriptorSet(renderer, descriptorSetLayout, structuredBuffer);
    let pipelineState = buildPipelineState(shaderProgram, renderer, pipelineLayout);
    printInitialValues(initialArray, 4);
    dispatchComputation(renderer, pipelineState, pipelineLayout, descriptorSet);
    print_output(renderer, structuredBuffer, 4);


    return true;
}
