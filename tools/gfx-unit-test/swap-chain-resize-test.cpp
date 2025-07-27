#if 0

// Duplicated: This test is similar to slang-rhi\tests\test-surface.cpp

#include "gfx-test-util.h"
#include "platform/window.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

using namespace rhi;

namespace gfx_test
{
struct Vertex
{
    float position[3];
};

static const int kVertexCount = 3;
static const Vertex kVertexData[kVertexCount] = {
    {0, 0, 1},
    {4, 0, 1},
    {0, 4, 1},
};

struct SwapchainResizeTest
{
    IDevice* device;
    UnitTestContext* context;

    ComPtr<platform::Window> window;
    ComPtr<ICommandQueue> queue;
    ComPtr<ISurface> surface;

    ComPtr<ITexture> swapchainImages[2];
    uint32_t swapchainImageCount = 2;
    Format desiredFormat = Format::RGBA8Unorm;

    ComPtr<IBuffer> vertexBuffer;
    ComPtr<IInputLayout> inputLayout;
    ComPtr<IRenderPipeline> pipeline;
    ComPtr<IShaderProgram> shaderProgram;
    ComPtr<IShaderObject> rootShaderObject;

    uint32_t width = 500;
    uint32_t height = 500;

    void init(IDevice* device, UnitTestContext* context)
    {
        this->device = device;
        this->context = context;
    }

    void createSwapchainAndResources()
    {
        // Create window
        platform::Application::init();
        platform::WindowDesc windowDesc;
        windowDesc.title = "";
        windowDesc.width = width;
        windowDesc.height = height;
        windowDesc.style = platform::WindowStyle::Default;
        window = platform::Application::createWindow(windowDesc);

        // Create surface
        WindowHandle windowHandle = WindowHandle::fromHwnd((void*)window->getNativeHandle().handleValues[0]);
        surface = device->createSurface(windowHandle);

        // Configure surface (swapchain)
        SurfaceConfig config = {};
        config.format = desiredFormat;
        config.width = width;
        config.height = height;
        config.desiredImageCount = swapchainImageCount;
        config.vsync = true;
        surface->configure(config);

        // Create vertex buffer
        BufferDesc vertexBufferDesc = {};
        vertexBufferDesc.size = sizeof(Vertex) * kVertexCount;
        vertexBufferDesc.memoryType = MemoryType::DeviceLocal;
        vertexBufferDesc.usage = BufferUsage::VertexBuffer;
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBuffer = device->createBuffer(vertexBufferDesc, kVertexData);

        // Input layout
        InputElementDesc inputElements[] = {
            {"POSITIONA", 0, Format::RGB32Float, offsetof(Vertex, position), 0},
        };
        VertexStreamDesc vertexStreams[] = {
            {sizeof(Vertex), InputSlotClass::PerVertex, 0},
        };
        InputLayoutDesc inputLayoutDesc = {};
        inputLayoutDesc.inputElementCount = sizeof(inputElements) / sizeof(InputElementDesc);
        inputLayoutDesc.inputElements = inputElements;
        inputLayoutDesc.vertexStreamCount = sizeof(vertexStreams) / sizeof(VertexStreamDesc);;
        inputLayoutDesc.vertexStreams = vertexStreams;

        GFX_CHECK_CALL_ABORT(device->createInputLayout(inputLayoutDesc, inputLayout.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection = nullptr;
        GFX_CHECK_CALL_ABORT(loadGraphicsProgram(
            device,
            shaderProgram,
            "swapchain-shader",
            "vertexMain",
            "fragmentMain",
            slangReflection
        ));


        // Pipeline
        ColorTargetDesc colorTarget = {};
        colorTarget.format = desiredFormat;
        RenderPipelineDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        pipelineDesc.inputLayout = inputLayout.get();
        pipelineDesc.primitiveTopology = PrimitiveTopology::TriangleList;
        pipelineDesc.targets = &colorTarget;
        pipelineDesc.targetCount = 1;
        pipeline = device->createRenderPipeline(pipelineDesc);
    }

    void renderFrame(uint32_t imageIndex)  
    {  
        // Acquire next image  
        ComPtr<ITexture> backBuffer;  
        if (SLANG_FAILED(surface->acquireNextImage(backBuffer.writeRef())))  
        {  
            return;  
        }  

        // Create command encoder  
        ComPtr<ICommandQueue> queue = device->getQueue(QueueType::Graphics);  
        ComPtr<ICommandEncoder> encoder = queue->createCommandEncoder();  

        // Render pass setup  
        RenderPassColorAttachment colorAttachment = {};  
        colorAttachment.view = backBuffer->getDefaultView();  
        colorAttachment.loadOp = LoadOp::Clear;  
        colorAttachment.storeOp = StoreOp::Store;  
        float clearColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};  
        memcpy(colorAttachment.clearValue, clearColor, sizeof(clearColor));  
        RenderPassDesc passDesc = {};  
        passDesc.colorAttachments = &colorAttachment;  
        passDesc.colorAttachmentCount = 1;  

        // Begin render pass  
        auto pass = encoder->beginRenderPass(passDesc);  

        // Bind pipeline and root object  
        pass->bindPipeline(pipeline, rootShaderObject);  

        // Set render state  
        RenderState state = {};  
        state.vertexBuffers[0] = BufferOffsetPair(vertexBuffer, 0);  
        state.vertexBufferCount = 1;  
        // Set viewport  
        Viewport viewport = Viewport::fromSize((float)width, (float)height);  
        state.viewportCount = 1;
        state.viewports[0] = viewport;

        pass->setRenderState(state);  

        // Draw  
        DrawArguments args = {};  
        args.vertexCount = kVertexCount;  
        pass->draw(args);  

        pass->end();  
        ComPtr<ICommandBuffer> cmdBuffer;  
        encoder->finish(cmdBuffer.writeRef());  
        queue->submit(cmdBuffer);  

        // Present  
        surface->present();  
    }

    void run()
    {
        createSwapchainAndResources();
        for (uint32_t i = 0; i < 5; ++i)
        {
            renderFrame(i % swapchainImageCount);
        }
        queue->waitOnHost();

        // Resize swapchain
        width = 700;
        height = 700;
        SurfaceConfig config = surface->getConfig();
        config.width = width;
        config.height = height;
        surface->configure(config);

        for (uint32_t i = 0; i < 5; ++i)
        {
            renderFrame(i % swapchainImageCount);
        }
        queue->waitOnHost();
    }
};

void swapchainResizeTestImpl(IDevice* device, UnitTestContext* context)
{
    SwapchainResizeTest t;
    t.init(device, context);
    t.run();
}

SLANG_UNIT_TEST(swapchainResizeD3D12)
{
    runTestImpl(swapchainResizeTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(swapchainResizeVulkan)
{
    runTestImpl(swapchainResizeTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test

#endif
