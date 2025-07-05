// gui.cpp
#include "gui.h"

#ifdef _WIN32
#include <examples/imgui_impl_win32.h>
#include <windows.h>
IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

using namespace rhi;

namespace platform
{

#ifdef _WIN32
LRESULT CALLBACK guiWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT handled = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    if (handled)
        return handled;
    ImGuiIO& io = ImGui::GetIO();

    switch (msg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        if (io.WantCaptureMouse)
            handled = 1;
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        if (io.WantCaptureKeyboard)
            handled = 1;
        break;
    }

    return handled;
}
#endif


GUI::GUI(Window* window, IDevice* inDevice, ICommandQueue* inQueue)
    : device(inDevice), queue(inQueue)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

#ifdef _WIN32
    ImGui_ImplWin32_Init((HWND)window->getNativeHandle().handleValues[0]);
#endif

    // Let's do the initialization work required for our graphics API
    // abstraction layer, so that we can pipe all IMGUI rendering
    // through the same interface as other work.
    //

    static const char* shaderCode = "cbuffer U { float4x4 mvp; };           \
    Texture2D t;                            \
    SamplerState s;                         \
    struct AssembledVertex {                \
        float2 pos;                         \
        float2 uv;                          \
        float4 col;                         \
    };                                      \
    struct CoarseVertex {                   \
        float4 col;                         \
        float2 uv;                          \
    };                                      \
    struct VSOutput {                       \
        CoarseVertex cv : U;                \
        float4 pos : SV_Position;           \
    };                                      \
    void vertexMain(                        \
        AssembledVertex i : U,              \
        out VSOutput    o)                  \
    {                                       \
        o.cv.col = i.col;                   \
        o.cv.uv = i.uv;                     \
        o.pos = mul(mvp,                    \
            float4(i.pos.xy, 0.f, 1.f));    \
    }                                       \
    float4 fragmentMain(                    \
        CoarseVertex     i : U)             \
        : SV_target                         \
    {                                       \
        return i.col * t.Sample(s, i.uv);   \
    }                                       \
    ";

    auto slangSession = inDevice->getSlangSession();

    // TODO: create slang program.
    // For now, we'll proceed without a proper shader program
    // This is a limitation that would need to be addressed for full functionality
#if 0
    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = slangGlobalScope;
    shaderProgram = device->createShaderProgram(programDesc);
#endif

    InputElementDesc inputElements[] = {
        {"U", 0, Format::RG32Float, offsetof(ImDrawVert, pos)},
        {"U", 1, Format::RG32Float, offsetof(ImDrawVert, uv)},
        {"U", 2, Format::RGBA8Unorm, offsetof(ImDrawVert, col)},
    };
    inputLayout = device->createInputLayout(
        sizeof(ImDrawVert),
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));

    // For now, skip pipeline creation since we don't have a shader program
    // This would need to be completed for full functionality
#if 0
    ColorTargetDesc colorTarget;
    colorTarget.format = Format::RGBA8Unorm;
    colorTarget.enableBlend = true;
    colorTarget.color.srcFactor = BlendFactor::SrcAlpha;
    colorTarget.color.dstFactor = BlendFactor::InvSrcAlpha;
    colorTarget.alpha.srcFactor = BlendFactor::InvSrcAlpha;
    colorTarget.alpha.dstFactor = BlendFactor::Zero;

    RenderPipelineDesc pipelineDesc;
    pipelineDesc.program = shaderProgram;
    pipelineDesc.inputLayout = inputLayout;
    pipelineDesc.targetCount = 1;
    pipelineDesc.targets = &colorTarget;
    pipelineDesc.rasterizer.cullMode = CullMode::None;
    pipelineDesc.depthStencil.depthTestEnable = false;
    pipelineDesc.primitiveTopology = PrimitiveTopology::TriangleList;

    pipelineState = device->createRenderPipeline(pipelineDesc);
#endif

    // Initialize the texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    {
        TextureDesc desc = {};
        desc.type = TextureType::Texture2D;
        desc.format = Format::RGBA8Unorm;
        desc.arrayLength = 1;
        desc.size.width = width;
        desc.size.height = height;
        desc.size.depth = 1;
        desc.mipCount = 1;
        desc.usage = TextureUsage::ShaderResource;
        desc.defaultState = ResourceState::ShaderResource;

        SubresourceData initData = {};
        initData.data = pixels;
        initData.rowPitch = width * 4 * sizeof(unsigned char);
        initData.slicePitch = initData.rowPitch * height;

        auto texture = device->createTexture(desc, &initData);

        TextureViewDesc viewDesc;
        viewDesc.format = desc.format;
        viewDesc.aspect = TextureAspect::All;
        auto textureView = device->createTextureView(texture, viewDesc);

        io.Fonts->TexID = (void*)textureView.detach();
    }

    {
        SamplerDesc desc;
        samplerState = device->createSampler(desc);
    }
}


void GUI::beginFrame()
{
#ifdef _WIN32
    ImGui_ImplWin32_NewFrame();
#endif
    ImGui::NewFrame();
}

void GUI::endFrame(ITexture* renderTarget)
{
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();
    auto vertexCount = draw_data->TotalVtxCount;
    auto indexCount = draw_data->TotalIdxCount;
    int commandListCount = draw_data->CmdListsCount;

    if (!vertexCount)
        return;
    if (!indexCount)
        return;
    if (!commandListCount)
        return;

        // For now, skip rendering since we don't have a complete pipeline
        // This would need shader program creation to work properly
#if 0
    // Create vertex and index buffers for this frame
    BufferDesc vertexBufferDesc;
    vertexBufferDesc.size = vertexCount * sizeof(ImDrawVert);
    vertexBufferDesc.usage = BufferUsage::VertexBuffer | BufferUsage::CopyDestination;
    vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
    vertexBufferDesc.memoryType = MemoryType::Upload;
    auto vertexBuffer = device->createBuffer(vertexBufferDesc);

    BufferDesc indexBufferDesc;
    indexBufferDesc.size = indexCount * sizeof(ImDrawIdx);
    indexBufferDesc.usage = BufferUsage::IndexBuffer | BufferUsage::CopyDestination;
    indexBufferDesc.defaultState = ResourceState::IndexBuffer;
    indexBufferDesc.memoryType = MemoryType::Upload;
    auto indexBuffer = device->createBuffer(indexBufferDesc);

    // Upload vertex and index data
    {
        void* vertexData;
        device->mapBuffer(vertexBuffer, CpuAccessMode::Write, &vertexData);
        size_t vertexOffset = 0;
        for (int ii = 0; ii < commandListCount; ++ii)
        {
            const ImDrawList* commandList = draw_data->CmdLists[ii];
            size_t dataSize = commandList->VtxBuffer.Size * sizeof(ImDrawVert);
            memcpy((char*)vertexData + vertexOffset, commandList->VtxBuffer.Data, dataSize);
            vertexOffset += dataSize;
        }
        device->unmapBuffer(vertexBuffer);

        void* indexData;
        device->mapBuffer(indexBuffer, CpuAccessMode::Write, &indexData);
        size_t indexOffset = 0;
        for (int ii = 0; ii < commandListCount; ++ii)
        {
            const ImDrawList* commandList = draw_data->CmdLists[ii];
            size_t dataSize = commandList->IdxBuffer.Size * sizeof(ImDrawIdx);
            memcpy((char*)indexData + indexOffset, commandList->IdxBuffer.Data, dataSize);
            indexOffset += dataSize;
        }
        device->unmapBuffer(indexBuffer);
    }

    // Create constant buffer for projection matrix
    BufferDesc constantBufferDesc;
    constantBufferDesc.size = sizeof(glm::mat4x4);
    constantBufferDesc.usage = BufferUsage::ConstantBuffer | BufferUsage::CopyDestination;
    constantBufferDesc.defaultState = ResourceState::ConstantBuffer;
    constantBufferDesc.memoryType = MemoryType::Upload;
    auto constantBuffer = device->createBuffer(constantBufferDesc);

    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };

        void* constantData;
        device->mapBuffer(constantBuffer, CpuAccessMode::Write, &constantData);
        memcpy(constantData, mvp, sizeof(mvp));
        device->unmapBuffer(constantBuffer);
    }

    // Record rendering commands
    auto commandEncoder = queue->createCommandEncoder();
    
    ComPtr<ITextureView> renderTargetView = device->createTextureView(renderTarget, {});
    RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = renderTargetView;
    colorAttachment.loadOp = LoadOp::Load;
    colorAttachment.storeOp = StoreOp::Store;

    RenderPassDesc renderPass = {};
    renderPass.colorAttachments = &colorAttachment;
    renderPass.colorAttachmentCount = 1;

    auto renderEncoder = commandEncoder->beginRenderPass(renderPass);

    RenderState renderState = {};
    renderState.viewports[0] = Viewport::fromSize(draw_data->DisplaySize.x, draw_data->DisplaySize.y);
    renderState.viewportCount = 1;
    renderState.vertexBuffers[0] = vertexBuffer;
    renderState.vertexBufferCount = 1;
    renderState.indexBuffer = indexBuffer;
    renderState.indexFormat = sizeof(ImDrawIdx) == 2 ? IndexFormat::Uint16 : IndexFormat::Uint32;

    auto rootObject = renderEncoder->bindPipeline(pipelineState);
    renderEncoder->setRenderState(renderState);

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    ImVec2 pos = draw_data->DisplayPos;
    for (int ii = 0; ii < commandListCount; ++ii)
    {
        auto commandList = draw_data->CmdLists[ii];
        auto commandCount = commandList->CmdBuffer.Size;
        for (int jj = 0; jj < commandCount; jj++)
        {
            auto command = &commandList->CmdBuffer[jj];
            if (auto userCallback = command->UserCallback)
            {
                userCallback(commandList, command);
            }
            else
            {
                ScissorRect rect = {
                    (uint32_t)(command->ClipRect.x - pos.x),
                    (uint32_t)(command->ClipRect.y - pos.y),
                    (uint32_t)(command->ClipRect.z - pos.x),
                    (uint32_t)(command->ClipRect.w - pos.y)};
                
                RenderState scissorState = renderState;
                scissorState.scissorRects[0] = rect;
                scissorState.scissorRectCount = 1;
                renderEncoder->setRenderState(scissorState);

                DrawArguments drawArgs = {};
                drawArgs.vertexCount = command->ElemCount;
                drawArgs.startIndexLocation = indexOffset;
                drawArgs.startVertexLocation = vertexOffset;
                renderEncoder->drawIndexed(drawArgs);
            }
            indexOffset += command->ElemCount;
        }
        vertexOffset += commandList->VtxBuffer.Size;
    }
    
    renderEncoder->end();
    queue->submit(commandEncoder->finish());
#endif
}

GUI::~GUI()
{
    auto& io = ImGui::GetIO();

    {
        Slang::ComPtr<ITextureView> textureView;
        textureView.attach((ITextureView*)io.Fonts->TexID);
        textureView = nullptr;
    }

#ifdef _WIN32
    ImGui_ImplWin32_Shutdown();
#endif

    ImGui::DestroyContext();
}

} // namespace platform

#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#ifdef _WIN32
  // imgui_impl_win32 defines these, so make sure it doesn't error because
// they're already there
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#include <examples/imgui_impl_win32.cpp>
#endif
