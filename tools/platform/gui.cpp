// gui.cpp
#include "gui.h"

#ifdef _WIN32
#include <windows.h>
#include "external/imgui/examples/imgui_impl_win32.h"
IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

using namespace gfx;

namespace platform
{

#ifdef _WIN32
LRESULT CALLBACK guiWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT handled = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    if(handled) return handled;
    ImGuiIO& io = ImGui::GetIO();

    switch( msg )
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        if(io.WantCaptureMouse) handled = 1;
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        if(io.WantCaptureKeyboard) handled = 1;
        break;
    }

    return handled;
}
#endif


GUI::GUI(
    Window* window,
    IDevice* inDevice,
    ICommandQueue* inQueue,
    IFramebufferLayout* framebufferLayout)
    : device(inDevice)
    , queue(inQueue)
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

    static const char* shaderCode =
    "cbuffer U { float4x4 mvp; };           \
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
    IShaderProgram* program = nullptr;
#if 0
    gfx::IShaderProgram::Desc programDesc = {};
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.slangProgram = slangProgram;
    program = device->createProgram(programDesc);
#endif
    InputElementDesc inputElements[] = {
        {"U", 0, Format::RG_Float32,        offsetof(ImDrawVert, pos) },
        {"U", 1, Format::RG_Float32,        offsetof(ImDrawVert, uv) },
        {"U", 2, Format::RGBA_Unorm_UInt8,  offsetof(ImDrawVert, col) },
    };
    auto inputLayout = device->createInputLayout(
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));

    //

    TargetBlendDesc targetBlendDesc;
    targetBlendDesc.color.srcFactor = BlendFactor::SrcAlpha;
    targetBlendDesc.color.dstFactor = BlendFactor::InvSrcAlpha;
    targetBlendDesc.alpha.srcFactor = BlendFactor::InvSrcAlpha;
    targetBlendDesc.alpha.dstFactor = BlendFactor::Zero;

    GraphicsPipelineStateDesc pipelineDesc;
    pipelineDesc.framebufferLayout = framebufferLayout;
    pipelineDesc.program = program;
    pipelineDesc.inputLayout = inputLayout;
    pipelineDesc.blend.targets = &targetBlendDesc;
    pipelineDesc.blend.targetCount = 1;
    pipelineDesc.rasterizer.cullMode = CullMode::None;

    // Set up the pieces of fixed-function state that we care about
    pipelineDesc.depthStencil.depthTestEnable = false;

    // TODO: need to set up blending state...

    pipelineState = device->createGraphicsPipelineState(pipelineDesc);

    // Initialize the texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    {
        gfx::ITextureResource::Desc desc;
        desc.init2D(IResource::Type::Texture2D, Format::RGBA_Unorm_UInt8, width, height, 1);
        desc.setDefaults(IResource::Usage::PixelShaderResource);

        ITextureResource::SubresourceData initData = {};
        initData.data = pixels;
        initData.strideY = width * 4 * sizeof(unsigned char);

        auto texture =
            device->createTextureResource(IResource::Usage::PixelShaderResource, desc, &initData);

        gfx::IResourceView::Desc viewDesc;
        viewDesc.format = desc.format;
        viewDesc.type = IResourceView::Type::ShaderResource;
        auto textureView = device->createTextureView(texture, viewDesc);

        io.Fonts->TexID = (void*) textureView.detach();
    }

    {
        ISamplerState::Desc desc;
        samplerState = device->createSamplerState(desc);
    }

    {
        IRenderPassLayout::Desc desc;
        desc.framebufferLayout = framebufferLayout;
        IRenderPassLayout::AttachmentAccessDesc colorAccess;
        desc.depthStencilAccess = nullptr;
        colorAccess.initialState = ResourceState::Present;
        colorAccess.finalState = ResourceState::Present;
        colorAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Load;
        colorAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
        desc.renderTargetAccess = &colorAccess;
        desc.renderTargetCount = 1;
        renderPass = device->createRenderPassLayout(desc);
    }
}



void GUI::beginFrame()
{
#ifdef _WIN32
    ImGui_ImplWin32_NewFrame();
#endif
    ImGui::NewFrame();
}

void GUI::endFrame(IFramebuffer* framebuffer)
{
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();
    auto vertexCount = draw_data->TotalVtxCount;
    auto indexCount = draw_data->TotalIdxCount;
    int commandListCount = draw_data->CmdListsCount;

    if(!vertexCount)        return;
    if(!indexCount)         return;
    if(!commandListCount)   return;

    // Allocate transient vertex/index buffers to hold the data for this frame.

    gfx::IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(vertexCount * sizeof(ImDrawVert));
    vertexBufferDesc.setDefaults(IResource::Usage::VertexBuffer);
    vertexBufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;
    auto vertexBuffer =
        device->createBufferResource(IResource::Usage::VertexBuffer, vertexBufferDesc);

    gfx::IBufferResource::Desc indexBufferDesc;
    indexBufferDesc.init(indexCount * sizeof(ImDrawIdx));
    indexBufferDesc.setDefaults(IResource::Usage::IndexBuffer);
    indexBufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;
    auto indexBuffer = device->createBufferResource(
        IResource::Usage::IndexBuffer,
        indexBufferDesc);
    auto cmdBuf = queue->createCommandBuffer();
    auto encoder = cmdBuf->encodeResourceCommands();
    {
        for(int ii = 0; ii < commandListCount; ++ii)
        {
            const ImDrawList* commandList = draw_data->CmdLists[ii];
            encoder->uploadBufferData(
                vertexBuffer,
                commandList->VtxBuffer.Size * ii * sizeof(ImDrawVert),
                commandList->VtxBuffer.Size * sizeof(ImDrawVert),
                commandList->VtxBuffer.Data);
            encoder->uploadBufferData(
                indexBuffer,
                commandList->IdxBuffer.Size * ii * sizeof(ImDrawIdx),
                commandList->IdxBuffer.Size * sizeof(ImDrawIdx),
                commandList->IdxBuffer.Data);
        }
    }

    // Allocate a transient constant buffer for projection matrix
    gfx::IBufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(sizeof(glm::mat4x4));
    constantBufferDesc.setDefaults(IResource::Usage::ConstantBuffer);
    constantBufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;
    auto constantBuffer =
        device->createBufferResource(IResource::Usage::ConstantBuffer, constantBufferDesc);

    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        encoder->uploadBufferData(constantBuffer, 0, sizeof(mvp), mvp);
    }

    encoder->endEncoding();

    gfx::Viewport viewport;
    viewport.originX = 0;
    viewport.originY = 0;
    viewport.extentY = draw_data->DisplaySize.y;
    viewport.extentX = draw_data->DisplaySize.x;
    viewport.extentY = draw_data->DisplaySize.y;
    viewport.minZ = 0;
    viewport.maxZ = 1;

    auto renderEncoder = cmdBuf->encodeRenderCommands(renderPass, framebuffer);
    renderEncoder->setViewportAndScissor(viewport);

    renderEncoder->setPipelineState(pipelineState);

    renderEncoder->setVertexBuffer(0, vertexBuffer, sizeof(ImDrawVert));
    renderEncoder->setIndexBuffer(
        indexBuffer, sizeof(ImDrawIdx) == 2 ? Format::R_UInt16 : Format::R_UInt32);
    renderEncoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    UInt vertexOffset = 0;
    UInt indexOffset = 0;
    ImVec2 pos = draw_data->DisplayPos;
    for(int ii = 0; ii < commandListCount; ++ii)
    {
        auto commandList = draw_data->CmdLists[ii];
        auto commandCount = commandList->CmdBuffer.Size;
        for(int jj = 0; jj < commandCount; jj++)
        {
            auto command = &commandList->CmdBuffer[jj];
            if(auto userCallback = command->UserCallback)
            {
                userCallback(commandList, command);
            }
            else
            {
                ScissorRect rect =
                {
                    (Int)(command->ClipRect.x - pos.x),
                    (Int)(command->ClipRect.y - pos.y),
                    (Int)(command->ClipRect.z - pos.x),
                    (Int)(command->ClipRect.w - pos.y)
                };
                renderEncoder->setScissorRects(1, &rect);

                // TODO: set parameter into root shader object.
                
                renderEncoder->drawIndexed(command->ElemCount, indexOffset, vertexOffset);
            }
            indexOffset += command->ElemCount;
        }
        vertexOffset += commandList->VtxBuffer.Size;
    }
    renderEncoder->endEncoding();
    cmdBuf->close();
    queue->executeCommandBuffer(cmdBuf);
}

GUI::~GUI()
{
    auto& io = ImGui::GetIO();

    {
        ComPtr<IResourceView> textureView;
        textureView.attach((IResourceView*) io.Fonts->TexID);
        textureView = nullptr;
    }

#ifdef _WIN32
     ImGui_ImplWin32_Shutdown();
#endif

     ImGui::DestroyContext();
}

} // gfx

#include "external/imgui/imgui.cpp"
#include "external/imgui/imgui_draw.cpp"
#ifdef _WIN32
#include "external/imgui/examples/imgui_impl_win32.cpp"
#endif
