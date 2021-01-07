// gui.cpp
#include "gui.h"

#ifdef _WIN32
#include <windows.h>
#include "external/imgui/examples/imgui_impl_win32.h"
IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif


namespace gfx {

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
void setNativeWindowHook(Window* window, WNDPROC proc);
#endif


GUI::GUI(Window* window, IRenderer* inRenderer)
    : renderer(inRenderer)
{
     ImGui::CreateContext();
     ImGuiIO& io = ImGui::GetIO();

#ifdef _WIN32
     ImGui_ImplWin32_Init(getPlatformWindowHandle(window));

     setNativeWindowHook(window, &guiWindowProc);
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

    SlangSession* slangSession = spCreateSession(nullptr);
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // TODO: These two lines need to change based on what the target graphics API
    // is, so we need a way for a `Renderer` to pass back its prefeerred code
    // format and profile name...
    //
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_4_0"));

    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, "gui.cpp.slang", shaderCode);

    char const* vertexEntryPointName    = "vertexMain";
    char const* fragmentEntryPointName  = "fragmentMain";
    int vertexIndex   = spAddEntryPoint(slangRequest, translationUnitIndex, vertexEntryPointName,   SLANG_STAGE_VERTEX);
    int fragmentIndex = spAddEntryPoint(slangRequest, translationUnitIndex, fragmentEntryPointName, SLANG_STAGE_FRAGMENT);

    const SlangResult compileRes = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        reportError("%s", diagnostics);
    }
    if(SLANG_FAILED(compileRes))
    {
        spDestroyCompileRequest(slangRequest);
        spDestroySession(slangSession);
        assert(!"unexpected");
        return;
    }

    ISlangBlob* vertexShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, vertexIndex, 0, &vertexShaderBlob);

    ISlangBlob* fragmentShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, fragmentIndex, 0, &fragmentShaderBlob);

    char const* vertexCode = (char const*) vertexShaderBlob->getBufferPointer();
    char const* vertexCodeEnd = vertexCode + vertexShaderBlob->getBufferSize();

    char const* fragmentCode = (char const*) fragmentShaderBlob->getBufferPointer();
    char const* fragmentCodeEnd = fragmentCode + fragmentShaderBlob->getBufferSize();

    spDestroyCompileRequest(slangRequest);
    spDestroySession(slangSession);

    gfx::ShaderProgram::KernelDesc kernelDescs[] =
    {
        { gfx::StageType::Vertex,    vertexCode,     vertexCodeEnd },
        { gfx::StageType::Fragment,  fragmentCode,   fragmentCodeEnd },
    };

    gfx::ShaderProgram::Desc programDesc;
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.kernels = &kernelDescs[0];
    programDesc.kernelCount = 2;

    auto program = renderer->createProgram(programDesc);

    vertexShaderBlob->release();
    fragmentShaderBlob->release();

    InputElementDesc inputElements[] = {
        {"U", 0, Format::RG_Float32,        offsetof(ImDrawVert, pos) },
        {"U", 1, Format::RG_Float32,        offsetof(ImDrawVert, uv) },
        {"U", 2, Format::RGBA_Unorm_UInt8,  offsetof(ImDrawVert, col) },
    };
    auto inputLayout = renderer->createInputLayout(
        &inputElements[0],
        SLANG_COUNT_OF(inputElements));

    //

    List<DescriptorSetLayout::SlotRangeDesc> descriptorSetRanges;
    descriptorSetRanges.add(DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::UniformBuffer));
    descriptorSetRanges.add(DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::SampledImage));
    descriptorSetRanges.add(DescriptorSetLayout::SlotRangeDesc(DescriptorSlotType::Sampler));

    DescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = descriptorSetRanges.getCount();
    descriptorSetLayoutDesc.slotRanges = descriptorSetRanges.getBuffer();

    descriptorSetLayout = renderer->createDescriptorSetLayout(descriptorSetLayoutDesc);

    List<PipelineLayout::DescriptorSetDesc> pipelineDescriptorSets;
    pipelineDescriptorSets.add(PipelineLayout::DescriptorSetDesc(descriptorSetLayout));

    PipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.descriptorSetCount = pipelineDescriptorSets.getCount();
    pipelineLayoutDesc.descriptorSets = pipelineDescriptorSets.getBuffer();
    pipelineLayoutDesc.renderTargetCount = 1;

    pipelineLayout = renderer->createPipelineLayout(pipelineLayoutDesc);

    TargetBlendDesc targetBlendDesc;
    targetBlendDesc.color.srcFactor = BlendFactor::SrcAlpha;
    targetBlendDesc.color.dstFactor = BlendFactor::InvSrcAlpha;
    targetBlendDesc.alpha.srcFactor = BlendFactor::InvSrcAlpha;
    targetBlendDesc.alpha.dstFactor = BlendFactor::Zero;

    GraphicsPipelineStateDesc pipelineDesc;
    pipelineDesc.renderTargetCount = 1;
    pipelineDesc.program = program;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.inputLayout = inputLayout;
    pipelineDesc.blend.targets = &targetBlendDesc;
    pipelineDesc.blend.targetCount = 1;
    pipelineDesc.rasterizer.cullMode = CullMode::None;

    // Set up the pieces of fixed-function state that we care about
    pipelineDesc.depthStencil.depthTestEnable = false;

    // TODO: need to set up blending state...

    pipelineState = renderer->createGraphicsPipelineState(pipelineDesc);

    // Initialize the texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    {
        gfx::TextureResource::Desc desc;
        desc.init2D(Resource::Type::Texture2D, Format::RGBA_Unorm_UInt8, width, height, 1);
        desc.setDefaults(Resource::Usage::PixelShaderResource);


        ptrdiff_t mipRowStrides[] = { ptrdiff_t(width * 4 * sizeof(unsigned char)) };
        void* subResourceData[] = { pixels };
        TextureResource::Data initData;
        initData.mipRowStrides = mipRowStrides;
        initData.numMips = 1;
        initData.numSubResources = 1;
        initData.subResources = subResourceData;

        auto texture = renderer->createTextureResource(Resource::Usage::PixelShaderResource, desc, &initData);

        gfx::ResourceView::Desc viewDesc;
        viewDesc.format = desc.format;
        viewDesc.type = ResourceView::Type::ShaderResource;
        auto textureView = renderer->createTextureView(texture, viewDesc);

        io.Fonts->TexID = (void*) textureView.detach();
    }

    {
        SamplerState::Desc desc;
        samplerState = renderer->createSamplerState(desc);
    }
}



void GUI::beginFrame()
{
#ifdef _WIN32
    ImGui_ImplWin32_NewFrame();
#endif
    ImGui::NewFrame();
}

void GUI::endFrame()
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

    gfx::BufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(vertexCount * sizeof(ImDrawVert));
    vertexBufferDesc.setDefaults(Resource::Usage::VertexBuffer);
    vertexBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;
    auto vertexBuffer = renderer->createBufferResource(
        Resource::Usage::VertexBuffer,
        vertexBufferDesc);

    gfx::BufferResource::Desc indexBufferDesc;
    indexBufferDesc.init(indexCount * sizeof(ImDrawIdx));
    indexBufferDesc.setDefaults(Resource::Usage::IndexBuffer);
    indexBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;
    auto indexBuffer = renderer->createBufferResource(
        Resource::Usage::IndexBuffer,
        indexBufferDesc);

    {
        ImDrawVert* dstVertex = (ImDrawVert*) renderer->map(vertexBuffer, MapFlavor::WriteDiscard);
        ImDrawIdx* dstIndex = (ImDrawIdx*) renderer->map(indexBuffer, MapFlavor::WriteDiscard);

        for(int ii = 0; ii < commandListCount; ++ii)
        {
            const ImDrawList* commandList = draw_data->CmdLists[ii];
            memcpy(dstVertex, commandList->VtxBuffer.Data, commandList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(dstIndex, commandList->IdxBuffer.Data, commandList->IdxBuffer.Size * sizeof(ImDrawIdx));
            dstVertex += commandList->VtxBuffer.Size;
            dstIndex += commandList->IdxBuffer.Size;
        }

        renderer->unmap(vertexBuffer);
        renderer->unmap(indexBuffer);
    }

    // Allocate a transient constant buffer for projection matrix
    gfx::BufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(sizeof(glm::mat4x4));
    constantBufferDesc.setDefaults(Resource::Usage::ConstantBuffer);
    constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;
    auto constantBuffer = renderer->createBufferResource(
        Resource::Usage::ConstantBuffer,
        constantBufferDesc);

    {
        glm::mat4x4* dstMVP = (glm::mat4x4*) renderer->map(constantBuffer, MapFlavor::WriteDiscard);

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
        memcpy(dstMVP, mvp, sizeof(mvp));

        renderer->unmap(constantBuffer);
    }

    gfx::Viewport viewport;
    viewport.originX = 0;
    viewport.originY = 0;
    viewport.extentY = draw_data->DisplaySize.y;
    viewport.extentX = draw_data->DisplaySize.x;
    viewport.extentY = draw_data->DisplaySize.y;
    viewport.minZ = 0;
    viewport.maxZ = 1;

    renderer->setViewport(viewport);

    auto pipelineType = PipelineType::Graphics;
    renderer->setPipelineState(pipelineType, pipelineState);

    renderer->setVertexBuffer(0, vertexBuffer, sizeof(ImDrawVert));
    renderer->setIndexBuffer(indexBuffer, sizeof(ImDrawIdx) == 2 ? Format::R_UInt16 : Format::R_UInt32);
    renderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

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
                renderer->setScissorRect(rect);

                // TODO: This should be a dynamic/transient descriptor set...
                auto descriptorSet = renderer->createDescriptorSet(descriptorSetLayout);
                descriptorSet->setConstantBuffer(0, 0, constantBuffer);
                descriptorSet->setResource(1, 0,
                    (gfx::ResourceView*) command->TextureId);
                descriptorSet->setSampler(2, 0,
                    samplerState);

                renderer->setDescriptorSet(
                    pipelineType,
                    pipelineLayout,
                    0,
                    descriptorSet);

                renderer->drawIndexed(command->ElemCount, indexOffset, vertexOffset);
            }
            indexOffset += command->ElemCount;
        }
        vertexOffset += commandList->VtxBuffer.Size;
    }
}

GUI::~GUI()
{
    auto& io = ImGui::GetIO();

    {
        RefPtr<ResourceView> textureView;
        textureView.attach((ResourceView*) io.Fonts->TexID);
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
