#include "slang-hgvm-texture-blitter.h"

#include <slang-rhi/shader-cursor.h>

namespace Slang::HGVM
{
namespace
{
SlangResult fail(String& outError, const char* message)
{
    outError = message;
    return SLANG_FAIL;
}

SlangResult failWithDiagnostics(
    String& outError,
    const char* action,
    SlangResult result,
    ISlangBlob* diagnostics)
{
    StringBuilder message;
    message << action;
    if (diagnostics && diagnostics->getBufferPointer() && diagnostics->getBufferSize())
    {
        message << ": ";
        message.append(
            static_cast<const char*>(diagnostics->getBufferPointer()),
            diagnostics->getBufferSize());
    }
    outError = message.produceString();
    return SLANG_FAILED(result) ? result : SLANG_FAIL;
}

} // namespace

/// Converts an arbitrary graph render target into the active surface format.
struct TextureBlitter::Impl
{
public:
    explicit Impl(rhi::IDevice* device)
        : m_device(device)
    {
    }

    SlangResult blit(rhi::ITexture* destination, rhi::ITexture* source, String& outError)
    {
        if (!destination || !source)
            return fail(outError, "HGVM blitting requires source and destination textures");
        SLANG_RETURN_ON_FAIL(ensurePipeline(destination->getDesc().format, outError));

        ComPtr<rhi::ITextureView> destinationView = destination->getDefaultView();
        ComPtr<rhi::ICommandQueue> queue = m_device->getQueue(rhi::QueueType::Graphics);
        ComPtr<rhi::ICommandEncoder> encoder = queue ? queue->createCommandEncoder() : nullptr;
        if (!destinationView || !encoder)
            return fail(outError, "RHI failed to prepare HGVM presentation");

        rhi::RenderPassColorAttachment attachment = {};
        attachment.view = destinationView;
        attachment.loadOp = rhi::LoadOp::DontCare;
        attachment.storeOp = rhi::StoreOp::Store;
        rhi::RenderPassDesc passDesc = {};
        passDesc.colorAttachments = &attachment;
        passDesc.colorAttachmentCount = 1;
        rhi::IRenderPassEncoder* pass = encoder->beginRenderPass(passDesc);
        if (!pass)
            return fail(outError, "RHI failed to begin the HGVM blit pass");

        rhi::IShaderObject* root = pass->bindPipeline(m_pipeline);
        const rhi::TextureDesc& destinationDesc = destination->getDesc();
        uint32_t outputSize[2] = {destinationDesc.size.width, destinationDesc.size.height};
        if (!root)
        {
            pass->end();
            return fail(outError, "RHI failed to bind the HGVM blit pipeline");
        }
        rhi::ShaderCursor cursor(root);
        if (SLANG_FAILED(cursor.getField("source").setBinding(rhi::Binding(source))) ||
            SLANG_FAILED(cursor.getField("outputSize").setData(outputSize, sizeof(outputSize))))
        {
            pass->end();
            return fail(outError, "RHI failed to bind the HGVM blitter");
        }

        rhi::RenderState state = {};
        state.viewports[0] = rhi::Viewport::fromSize(float(outputSize[0]), float(outputSize[1]));
        state.viewportCount = 1;
        state.scissorRects[0] = rhi::ScissorRect::fromSize(outputSize[0], outputSize[1]);
        state.scissorRectCount = 1;
        pass->setRenderState(state);
        rhi::DrawArguments draw = {};
        draw.vertexCount = 3;
        pass->draw(draw);
        pass->end();

        ComPtr<rhi::ICommandBuffer> commandBuffer = encoder->finish();
        if (!commandBuffer || SLANG_FAILED(queue->submit(commandBuffer)))
            return fail(outError, "RHI failed to submit the HGVM blit");
        return SLANG_OK;
    }

    SlangResult ensurePipeline(rhi::Format format, String& outError)
    {
        if (m_pipeline && m_format == format)
            return SLANG_OK;
        if (!m_device)
            return fail(outError, "HGVM presentation requires an RHI device");

        if (!m_component)
        {
            static const char kBlitSource[] = R"(
struct BlitVarying
{
    float4 position : SV_Position;
};

[shader("vertex")]
BlitVarying vertexMain(uint vertexId : SV_VertexID)
{
    float2 position = vertexId == 0 ? float2(-1.0, -1.0)
        : vertexId == 1 ? float2(-1.0, 3.0)
                        : float2(3.0, -1.0);
    BlitVarying result;
    result.position = float4(position, 0.0, 1.0);
    return result;
}

[shader("fragment")]
float4 fragmentMain(
    BlitVarying input,
    Texture2D<float4> source,
    uniform uint2 outputSize) : SV_Target
{
    uint sourceWidth;
    uint sourceHeight;
    source.GetDimensions(sourceWidth, sourceHeight);
    uint2 pixel = min(
        uint2(input.position.xy) * uint2(sourceWidth, sourceHeight) / outputSize,
        uint2(sourceWidth - 1, sourceHeight - 1));
    return source.Load(int3(pixel, 0));
}
)";
            ComPtr<slang::ISession> session = m_device->getSlangSession();
            if (!session)
                return fail(outError, "RHI device has no Slang session");
            ComPtr<ISlangBlob> diagnostics;
            slang::IModule* module = session->loadModuleFromSourceString(
                "__slang_hgvm_blit",
                "__slang_hgvm_blit.slang",
                kBlitSource,
                diagnostics.writeRef());
            if (!module)
                return failWithDiagnostics(
                    outError,
                    "Failed to compile the blitter used by HGVM",
                    SLANG_FAIL,
                    diagnostics);

            ComPtr<slang::IEntryPoint> vertex;
            diagnostics.setNull();
            SlangResult result = module->findAndCheckEntryPoint(
                "vertexMain",
                SLANG_STAGE_VERTEX,
                vertex.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !vertex)
                return failWithDiagnostics(
                    outError,
                    "Failed to find the HGVM blit vertex entry point",
                    result,
                    diagnostics);

            ComPtr<slang::IEntryPoint> fragment;
            diagnostics.setNull();
            result = module->findAndCheckEntryPoint(
                "fragmentMain",
                SLANG_STAGE_FRAGMENT,
                fragment.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !fragment)
                return failWithDiagnostics(
                    outError,
                    "Failed to find the HGVM blit fragment entry point",
                    result,
                    diagnostics);

            slang::IComponentType* components[] = {module, vertex, fragment};
            ComPtr<slang::IComponentType> composite;
            diagnostics.setNull();
            result = session->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                composite.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !composite)
                return failWithDiagnostics(
                    outError,
                    "Failed to compose the HGVM blitter",
                    result,
                    diagnostics);
            diagnostics.setNull();
            result = composite->link(m_component.writeRef(), diagnostics.writeRef());
            if (SLANG_FAILED(result) || !m_component)
                return failWithDiagnostics(
                    outError,
                    "Failed to link the HGVM blitter",
                    result,
                    diagnostics);
        }

        if (!m_program)
        {
            ComPtr<ISlangBlob> diagnostics;
            m_program = m_device->createShaderProgram(m_component, diagnostics.writeRef());
            if (!m_program)
                return failWithDiagnostics(
                    outError,
                    "Failed to create the HGVM blit program",
                    SLANG_FAIL,
                    diagnostics);
        }

        if (!m_inputLayout)
        {
            rhi::InputLayoutDesc inputLayoutDesc = {};
            m_inputLayout = m_device->createInputLayout(inputLayoutDesc);
            if (!m_inputLayout)
                return fail(outError, "RHI failed to create the HGVM blitter input layout");
        }
        rhi::ColorTargetDesc target = {};
        target.format = format;
        rhi::RenderPipelineDesc desc = {};
        desc.program = m_program;
        desc.inputLayout = m_inputLayout;
        desc.compilationPolicy = rhi::PipelineCompilationPolicy::Immediate;
        desc.primitiveTopology = rhi::PrimitiveTopology::TriangleList;
        desc.targets = &target;
        desc.targetCount = 1;
        desc.depthStencil.depthTestEnable = false;
        desc.depthStencil.depthWriteEnable = false;
        desc.label = "HGVM surface blitter";
        m_pipeline = m_device->createRenderPipeline(desc);
        if (!m_pipeline)
            return fail(outError, "RHI failed to create the HGVM blit pipeline");
        m_format = format;
        return SLANG_OK;
    }

    ComPtr<rhi::IDevice> m_device;
    ComPtr<slang::IComponentType> m_component;
    ComPtr<rhi::IShaderProgram> m_program;
    ComPtr<rhi::IInputLayout> m_inputLayout;
    ComPtr<rhi::IRenderPipeline> m_pipeline;
    rhi::Format m_format = rhi::Format::Undefined;
};

TextureBlitter::TextureBlitter(rhi::IDevice* device)
    : m_impl(new Impl(device))
{
}

TextureBlitter::~TextureBlitter() = default;

SlangResult TextureBlitter::prepare(rhi::Format format, String& outError)
{
    return m_impl->ensurePipeline(format, outError);
}

SlangResult TextureBlitter::blit(
    rhi::ITexture* destination,
    rhi::ITexture* source,
    String& outError)
{
    return m_impl->blit(destination, source, outError);
}
} // namespace Slang::HGVM
