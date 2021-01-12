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
#include "slang-com-ptr.h"
#include "gfx/render.h"
#include "gfx/d3d11/render-d3d11.h"
#include "tools/graphics-app-framework/window.h"
#include "../../prelude/slang-cpp-types.h"
using namespace gfx;

// We create global ref pointers to avoid dereferencing values
//
RefPtr<gfx::ShaderProgram>         gShaderProgram;
RefPtr<gfx::ApplicationContext>    gAppContext;
Slang::ComPtr<gfx::IRenderer>      gRenderer;

RefPtr<gfx::BufferResource>        gStructuredBuffer;

RefPtr<gfx::PipelineLayout>        gPipelineLayout;
RefPtr<gfx::PipelineState>         gPipelineState;
RefPtr<gfx::DescriptorSetLayout>   gDescriptorSetLayout;
RefPtr<gfx::DescriptorSet>         gDescriptorSet;

// Boilerplate types to help the slan-generated file
//
struct gfx_Window_0;
struct gfx_Renderer_0;
struct gfx_BufferResource_0;
struct gfx_ShaderProgram_0;
struct gfx_DescriptorSetLayout_0;
struct gfx_PipelineLayout_0;
struct gfx_DescriptorSet_0;
struct gfx_PipelineState_0;

bool executeComputation_0();
extern unsigned char __computeMain[];
extern size_t __computeMainSize;

gfx::ShaderProgram* loadShaderProgram(gfx::IRenderer* renderer, unsigned char computeCode[], size_t computeCodeSize)
{
    // We extract the begin/end pointers to the output code buffers directly
    //
    char unsigned const* computeCodeEnd = computeCode + computeCodeSize;

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
    programDesc.kernelCount = 1;

    gShaderProgram = renderer->createProgram(programDesc);

    return gShaderProgram;
}

// Now that we've covered the function that actually loads and
// compiles our Slang shade code, we can go through the rest
// of the application code without as much commentary.
//
gfx::Window* createWindow(int windowWidth, int windowHeight)
{
    // Create a window for our application to render into.
    //
    WindowDesc windowDesc;
    windowDesc.title = "Hello, World!";
    windowDesc.width = windowWidth;
    windowDesc.height = windowHeight;
    return createWindow(windowDesc);
    //return globalWindow;
}

gfx::IRenderer* createRenderer(
    int windowWidth,
    int windowHeight,
    gfx::Window* window)
{
    // Initialize the rendering layer.
    //
    // Note: for now we are hard-coding logic to use the
    // Direct3D11 back-end for the graphics API abstraction.
    // A future version of this example may support multiple
    // platforms/APIs.
    //
    createD3D11Renderer(gRenderer.writeRef());
    IRenderer::Desc rendererDesc;
    rendererDesc.width = windowWidth;
    rendererDesc.height = windowHeight;
    {
        Result res = gRenderer->initialize(rendererDesc, getPlatformWindowHandle(window));
        if (SLANG_FAILED(res)) return nullptr;
    }
    return gRenderer;
}

gfx::BufferResource* createStructuredBuffer(gfx::IRenderer* renderer, float* initialArray)
{
    // Create a structured buffer for storing the data for computation
    //
    int structuredBufferSize = 4 * sizeof(float);

    BufferResource::Desc structuredBufferDesc;
    structuredBufferDesc.init(structuredBufferSize);
    structuredBufferDesc.setDefaults(Resource::Usage::UnorderedAccess);
    structuredBufferDesc.elementSize = 4;
    structuredBufferDesc.cpuAccessFlags = Resource::AccessFlag::Read;

    gStructuredBuffer = renderer->createBufferResource(
        Resource::Usage::UnorderedAccess,
        structuredBufferDesc,
        initialArray);
    return gStructuredBuffer;
}

gfx::DescriptorSetLayout* buildDescriptorSetLayout(gfx::IRenderer* renderer)
{
    // Our example graphics API usess a "modern" D3D12/Vulkan style
    // of resource binding, so now we will dive into describing and
    // allocating "descriptor sets."
    //
    // First, we need to construct a descriptor set *layout*.
    //
    DescriptorSetLayout::SlotRangeDesc slotRanges[] =
    {
        DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::StorageBuffer),
    };
    DescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = 1;
    descriptorSetLayoutDesc.slotRanges = &slotRanges[0];
    gDescriptorSetLayout = renderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    return gDescriptorSetLayout;
}

gfx::PipelineLayout* buildPipeline(gfx::IRenderer* renderer, gfx::DescriptorSetLayout* descriptorSetLayout)
{
    // Next we will allocate a pipeline layout, which specifies
    // that we will render with only a single descriptor set bound.
    //

    PipelineLayout::DescriptorSetDesc descriptorSets[] =
    {
        PipelineLayout::DescriptorSetDesc(descriptorSetLayout),
    };
    PipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.renderTargetCount = 1;
    pipelineLayoutDesc.descriptorSetCount = 1;
    pipelineLayoutDesc.descriptorSets = &descriptorSets[0];
    gPipelineLayout = renderer->createPipelineLayout(pipelineLayoutDesc);

    return gPipelineLayout;
}

gfx::DescriptorSet* buildDescriptorSet(
    gfx::IRenderer* renderer,
    gfx::DescriptorSetLayout* descriptorSetLayout,
    gfx::BufferResource* structuredBuffer)
{
    // Once we have the descriptor set layout, we can allocate
    // and fill in a descriptor set to hold our parameters.
    //
    gDescriptorSet = renderer->createDescriptorSet(descriptorSetLayout);
    if(!gDescriptorSet) return nullptr;

    // Once we have the bufferResource created, we can fill in
    // a descriptor set for creating a structured buffer
    //
    ResourceView::Desc resourceViewDesc;
    resourceViewDesc.type = ResourceView::Type::UnorderedAccess;
    auto resourceView = renderer->createBufferView(structuredBuffer, resourceViewDesc);
    gDescriptorSet->setResource(0, 0, resourceView);

    return gDescriptorSet;
}

gfx::PipelineState* buildPipelineState(
    gfx::ShaderProgram* shaderProgram,
    gfx::IRenderer* renderer,
    gfx::PipelineLayout* pipelineLayout)
{
    // Following the D3D12/Vulkan style of API, we need a pipeline state object
    // (PSO) to encapsulate the configuration of the overall graphics pipeline.
    //
    ComputePipelineStateDesc desc;
    desc.pipelineLayout = pipelineLayout;
    desc.program = shaderProgram;
    gPipelineState = renderer->createComputePipelineState(desc);
    return gPipelineState;
}

void printInitialValues(float* initialArray, int length)
{
    // Print out the values before the computation
    printf("Before:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%f, ", initialArray[i]);
    }
    printf("\n");
}

void dispatchComputation(
    gfx::IRenderer* gRenderer,
    gfx::PipelineState* gPipelineState,
    gfx::PipelineLayout* gPipelineLayout,
    gfx::DescriptorSet* gDescriptorSet,
    unsigned int gridDimsX,
    unsigned int gridDimsY,
    unsigned int gridDimsZ)
{

    gRenderer->setPipelineState(PipelineType::Compute, gPipelineState);
    gRenderer->setDescriptorSet(PipelineType::Compute, gPipelineLayout, 0, gDescriptorSet);

    gRenderer->dispatchCompute(gridDimsX, gridDimsY, gridDimsZ);
}

void print_output(
    gfx::IRenderer* renderer,
    gfx::BufferResource* structuredBuffer,
    int length)
{
    if (float* outputData = (float*)renderer->map(structuredBuffer, MapFlavor::HostRead))
    {
        // Print out the values the the kernel produced
        printf("After: \n");
        for (int i = 0; i < 4; i++)
        {
            printf("%f, ", outputData[i]);
        }
        printf("\n");

        renderer->unmap(structuredBuffer);
    }
}

// Boilerplate functions to help the slang-generated file and types
gfx_Window_0* createWindow_0(int32_t _0, int32_t _1)
{
    return (gfx_Window_0*)createWindow(_0, _1);
}

gfx_Renderer_0* createRenderer_0(int32_t _0, int32_t _1, gfx_Window_0* _2)
{
    return (gfx_Renderer_0*)createRenderer(_0, _1, (gfx::Window*)_2);
}

gfx_BufferResource_0* createStructuredBuffer_0(gfx_Renderer_0* _0, FixedArray<float, 4> _1)
{
    return (gfx_BufferResource_0*)createStructuredBuffer((gfx::IRenderer*)_0, (float*)&_1);
}

gfx_ShaderProgram_0* loadShaderProgram_0(gfx_Renderer_0* _0, unsigned char _1[], size_t _2)
{
    return (gfx_ShaderProgram_0*)loadShaderProgram((gfx::IRenderer*)_0, _1, _2);
}

gfx_DescriptorSetLayout_0* buildDescriptorSetLayout_0(gfx_Renderer_0* _0)
{
    return (gfx_DescriptorSetLayout_0*)buildDescriptorSetLayout((gfx::IRenderer*)_0);
}

gfx_PipelineLayout_0* buildPipeline_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1)
{
    return (gfx_PipelineLayout_0*)buildPipeline((gfx::IRenderer*)_0, (gfx::DescriptorSetLayout*)_1);
}

gfx_DescriptorSet_0* buildDescriptorSet_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1, gfx_BufferResource_0* _2)
{
    return (gfx_DescriptorSet_0*)buildDescriptorSet(
        (gfx::IRenderer*)_0,
        (gfx::DescriptorSetLayout*)_1,
        (gfx::BufferResource*)_2);
}

gfx_PipelineState_0* buildPipelineState_0(gfx_ShaderProgram_0* _0, gfx_Renderer_0* _1, gfx_PipelineLayout_0* _2)
{
    return (gfx_PipelineState_0*)buildPipelineState(
        (gfx::ShaderProgram*)_0, (gfx::IRenderer*)_1,
        (gfx::PipelineLayout*)_2);
}

void printInitialValues_0(FixedArray<float, 4> _0, int32_t _1)
{
    printInitialValues((float*)&_0, _1);
}

void dispatchComputation_0(gfx_Renderer_0* _0, gfx_PipelineState_0* _1, gfx_PipelineLayout_0* _2, gfx_DescriptorSet_0* _3, unsigned int gridDimsX, unsigned int gridDimsY, unsigned int gridDimsZ)
{
    dispatchComputation(
        (gfx::IRenderer*)_0,
        (gfx::PipelineState*)_1,
        (gfx::PipelineLayout*)_2,
        (gfx::DescriptorSet*)_3,
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

void print_output_0(gfx_Renderer_0* _0, gfx_BufferResource_0* _1, int32_t _2)
{
    print_output((gfx::IRenderer*)_0, (gfx::BufferResource*)_1, _2);
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

    if (!(executeComputation_0()))
    {
        return exitApplication(context, 1);
    }
}

// This macro instantiates an appropriate main function to
// invoke the `innerMain` above.
//
GFX_CONSOLE_MAIN(innerMain)
