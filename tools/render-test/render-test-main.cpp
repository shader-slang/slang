// render-test-main.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "options.h"
#include "slang-gfx.h"
#include "tools/gfx-util/shader-cursor.h"
#include "slang-support.h"
#include "png-serialize-util.h"

#include "shader-renderer-util.h"

#include "../source/core/slang-io.h"
#include "../source/core/slang-string-util.h"

#include "core/slang-token-reader.h"

#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#include "window.h"

#include "../../source/core/slang-test-tool-util.h"

#define ENABLE_RENDERDOC_INTEGRATION 0

#if ENABLE_RENDERDOC_INTEGRATION
#    include "external/renderdoc_app.h"
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#endif

namespace renderer_test {

using Slang::Result;

int gWindowWidth = 1024;
int gWindowHeight = 768;

//
// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//

struct Vertex
{
    float position[3];
    float color[3];
    float uv[2];
};

static const Vertex kVertexData[] =
{
    { { 0,  0, 0.5 }, {1, 0, 0} , {0, 0} },
    { { 0,  1, 0.5 }, {0, 0, 1} , {1, 0} },
    { { 1,  0, 0.5 }, {0, 1, 0} , {1, 1} },
};
static const int kVertexCount = SLANG_COUNT_OF(kVertexData);

using namespace Slang;

static void _outputProfileTime(uint64_t startTicks, uint64_t endTicks)
{
    WriterHelper out = StdWriters::getOut();
    double time = double(endTicks - startTicks) / ProcessUtil::getClockFrequency();
    out.print("profile-time=%g\n", time);
}

class ProgramVars;

struct ShaderOutputPlan
{
    struct Item
    {
        ComPtr<IResource>               resource;
        slang::TypeLayoutReflection*    typeLayout = nullptr;
    };

    List<Item> items;
};

class RenderTestApp
{
public:
    Result update();

    // At initialization time, we are going to load and compile our Slang shader
    // code, and then create the API objects we need for rendering.
    Result initialize(
        SlangSession* session,
        IDevice* device,
        const Options& options,
        const ShaderCompilerUtil::Input& input);
    void runCompute(IComputeCommandEncoder* encoder);
    void renderFrame(IRenderCommandEncoder* encoder);
    void finalize();

    void applyBinding(PipelineType pipelineType, ICommandEncoder* encoder);
    void setProjectionMatrix(IResourceCommandEncoder* encoder);
    Result writeBindingOutput(const char* fileName);

    Result writeScreen(const char* filename);

protected:
    /// Called in initialize
    Result _initializeShaders(
        SlangSession* session,
        IDevice* device,
        Options::ShaderProgramType shaderType,
        const ShaderCompilerUtil::Input& input);
    void _initializeRenderPass();

    uint64_t m_startTicks;

    // variables for state to be used for rendering...
    uintptr_t m_constantBufferSize;

    ComPtr<IDevice> m_device;
    ComPtr<ICommandQueue> m_queue;
    ComPtr<ITransientResourceHeap> m_transientHeap;
    ComPtr<IRenderPassLayout> m_renderPass;
    ComPtr<IInputLayout> m_inputLayout;
    ComPtr<IBufferResource> m_vertexBuffer;
    ComPtr<IShaderProgram> m_shaderProgram;
    ComPtr<IPipelineState> m_pipelineState;
    ComPtr<IFramebufferLayout> m_framebufferLayout;
    ComPtr<IFramebuffer> m_framebuffer;
    ComPtr<ITextureResource> m_colorBuffer;

    ShaderCompilerUtil::OutputAndLayout m_compilationOutput;

    ShaderInputLayout m_shaderInputLayout; ///< The binding layout

    Options m_options;

    ComPtr<IShaderObject> m_programVars;
    ShaderOutputPlan m_outputPlan;
};

struct AssignValsFromLayoutContext
{
    IDevice*                device;
    ShaderOutputPlan&       outputPlan;
    slang::ProgramLayout*   slangReflection;

    AssignValsFromLayoutContext(
        IDevice*                    device,
        ShaderOutputPlan&           outputPlan,
        slang::ProgramLayout*       slangReflection)
        : device(device)
        , outputPlan(outputPlan)
        , slangReflection(slangReflection)
    {}

    void maybeAddOutput(ShaderCursor const& dstCursor, ShaderInputLayout::Val* srcVal, IResource* resource)
    {
        if(srcVal->isOutput)
        {
            ShaderOutputPlan::Item item;
            item.resource = resource;
            item.typeLayout = dstCursor.getTypeLayout();
            outputPlan.items.add(item);
        }
    }

    SlangResult assignData(ShaderCursor const& dstCursor, ShaderInputLayout::DataVal* srcVal)
    {
        const size_t bufferSize = srcVal->bufferData.getCount() * sizeof(uint32_t);

        ShaderCursor dataCursor = dstCursor;
        switch(dataCursor.getTypeLayout()->getKind())
        {
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
            dataCursor = dataCursor.getDereferenced();
            break;

        default:
            break;

        }

        SLANG_RETURN_ON_FAIL(dataCursor.setData(srcVal->bufferData.getBuffer(), bufferSize));
        return SLANG_OK;
    }

    SlangResult assignBuffer(ShaderCursor const& dstCursor, ShaderInputLayout::BufferVal* srcVal)
    {
        const InputBufferDesc& srcBuffer = srcVal->bufferDesc;
        auto& bufferData = srcVal->bufferData;
        const size_t bufferSize = bufferData.getCount() * sizeof(uint32_t);

        ComPtr<IBufferResource> bufferResource;
        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBufferResource(srcBuffer, /*entry.isOutput,*/ bufferSize, bufferData.getBuffer(), device, bufferResource));

        IResourceView::Desc viewDesc;
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = srcBuffer.format;
        auto bufferView = device->createBufferView(
            bufferResource,
            viewDesc);
        dstCursor.setResource(bufferView);
        maybeAddOutput(dstCursor, srcVal, bufferResource);

        return SLANG_OK;
    }

    SlangResult assignCombinedTextureSampler(ShaderCursor const& dstCursor, ShaderInputLayout::CombinedTextureSamplerVal* srcVal)
    {
        auto& textureEntry = srcVal->textureVal;
        auto& samplerEntry = srcVal->samplerVal;

        ComPtr<ITextureResource> texture;
        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(
            textureEntry->textureDesc, textureBindFlags, device, texture));

        auto sampler = _createSamplerState(device, samplerEntry->samplerDesc);

        IResourceView::Desc viewDesc;
        viewDesc.type = IResourceView::Type::ShaderResource;
        auto textureView = device->createTextureView(
            texture,
            viewDesc);

        dstCursor.setCombinedTextureSampler(textureView, sampler);
        maybeAddOutput(dstCursor, srcVal, texture);

        return SLANG_OK;
    }

    static const int textureBindFlags = IResource::BindFlag::NonPixelShaderResource | IResource::BindFlag::PixelShaderResource;

    SlangResult assignTexture(ShaderCursor const& dstCursor, ShaderInputLayout::TextureVal* srcVal)
    {
        ComPtr<ITextureResource> texture;
        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(
            srcVal->textureDesc, textureBindFlags, device, texture));

        // TODO: support UAV textures...

        IResourceView::Desc viewDesc;
        viewDesc.type = IResourceView::Type::ShaderResource;
        viewDesc.format = texture->getDesc()->format;
        auto textureView = device->createTextureView(
            texture,
            viewDesc);

        if (!textureView)
        {
            return SLANG_FAIL;
        }

        dstCursor.setResource(textureView);
        maybeAddOutput(dstCursor, srcVal, texture);
        return SLANG_OK;
    }

    SlangResult assignSampler(ShaderCursor const& dstCursor, ShaderInputLayout::SamplerVal* srcVal)
    {
        auto sampler = _createSamplerState(device, srcVal->samplerDesc);

        dstCursor.setSampler(sampler);
        return SLANG_OK;
    }

    SlangResult assignAggregate(ShaderCursor const& dstCursor, ShaderInputLayout::AggVal* srcVal)
    {
        Index fieldCount = srcVal->fields.getCount();
        for(Index fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
        {
            auto& field = srcVal->fields[fieldIndex];

            if(field.name.getLength() == 0)
            {
                StdWriters::getError().print("error: entries in `ShaderInputLayout` must include a name\n");
                return SLANG_E_INVALID_ARG;
            }

            auto fieldCursor = dstCursor.getPath(field.name.getBuffer());

            if(!fieldCursor.isValid())
            {
                StdWriters::getError().print("error: could not find shader parameter matching '%s'\n", field.name.begin());
                return SLANG_E_INVALID_ARG;
            }

            assign(fieldCursor, field.val);
        }
        return SLANG_OK;
    }

    SlangResult assignObject(ShaderCursor const& dstCursor, ShaderInputLayout::ObjectVal* srcVal)
    {
        auto typeName = srcVal->typeName;
        slang::TypeReflection* slangType = nullptr;
        if(typeName.getLength() != 0)
        {
            // If the input line specified the name of the type
            // to allocate, then we use it directly.
            //
            slangType = slangReflection->findTypeByName(typeName.getBuffer());
        }
        else
        {
            // if the user did not specify what type to allocate,
            // then we will infer the type from the type of the
            // value pointed to by `entryCursor`.
            //
            auto slangTypeLayout = dstCursor.getTypeLayout();
            switch(slangTypeLayout->getKind())
            {
            default:
                break;

            case slang::TypeReflection::Kind::ConstantBuffer:
            case slang::TypeReflection::Kind::ParameterBlock:
                // If the cursor is pointing at a constant buffer
                // or parameter block, then we assume the user
                // actually means to allocate an object based on
                // the element type of the block.
                //
                slangTypeLayout = slangTypeLayout->getElementTypeLayout();
                break;
            }
            slangType = slangTypeLayout->getType();
        }

        ComPtr<IShaderObject> shaderObject = device->createShaderObject(slangType);

        assign(ShaderCursor(shaderObject), srcVal->contentVal);

        dstCursor.setObject(shaderObject);
        return SLANG_OK;
    }

    SlangResult assignArray(ShaderCursor const& dstCursor, ShaderInputLayout::ArrayVal* srcVal)
    {
        Index elementCounter = 0;
        for(auto elementVal : srcVal->vals)
        {
            Index elementIndex = elementCounter++;
            SLANG_RETURN_ON_FAIL(assign(dstCursor[elementIndex], elementVal));
        }
        return SLANG_OK;
    }

    SlangResult assign(ShaderCursor const& dstCursor, ShaderInputLayout::ValPtr const& srcVal)
    {
        auto& entryCursor = dstCursor;
        switch(srcVal->kind)
        {
        case ShaderInputType::UniformData:
            return assignData(dstCursor, (ShaderInputLayout::DataVal*) srcVal.Ptr());

        case ShaderInputType::Buffer:
            return assignBuffer(dstCursor, (ShaderInputLayout::BufferVal*) srcVal.Ptr());

        case ShaderInputType::CombinedTextureSampler:
            return assignCombinedTextureSampler(dstCursor, (ShaderInputLayout::CombinedTextureSamplerVal*) srcVal.Ptr());

        case ShaderInputType::Texture:
            return assignTexture(dstCursor, (ShaderInputLayout::TextureVal*) srcVal.Ptr());

        case ShaderInputType::Sampler:
            return assignSampler(dstCursor, (ShaderInputLayout::SamplerVal*) srcVal.Ptr());

        case ShaderInputType::Object:
            return assignObject(dstCursor, (ShaderInputLayout::ObjectVal*) srcVal.Ptr());

        case ShaderInputType::Aggregate:
            return assignAggregate(dstCursor, (ShaderInputLayout::AggVal*) srcVal.Ptr());

        case ShaderInputType::Array:
            return assignArray(dstCursor, (ShaderInputLayout::ArrayVal*) srcVal.Ptr());

        default:
            assert(!"Unhandled type");
            return SLANG_FAIL;
        }
    }
};

SlangResult _assignVarsFromLayout(
    IDevice*                   device,
    IShaderObject*               shaderObject,
    ShaderInputLayout const&    layout,
    ShaderOutputPlan&           ioOutputPlan,
    slang::ProgramLayout*       slangReflection)
{
    AssignValsFromLayoutContext context(device, ioOutputPlan, slangReflection);
    ShaderCursor rootCursor = ShaderCursor(shaderObject);
    return context.assign(rootCursor, layout.rootVal);
}

void RenderTestApp::applyBinding(PipelineType pipelineType, ICommandEncoder* encoder)
{
    switch (pipelineType)
    {
    case PipelineType::Compute:
        {
            ComPtr<IComputeCommandEncoder> computeEncoder;
            encoder->queryInterface(
                SLANG_UUID_IComputeCommandEncoder, (void**)computeEncoder.writeRef());
            computeEncoder->bindRootShaderObject(m_programVars);
        }
        break;
    case PipelineType::Graphics:
        {
            ComPtr<IRenderCommandEncoder> renderEncoder;
            encoder->queryInterface(
                SLANG_UUID_IRenderCommandEncoder, (void**)renderEncoder.writeRef());
            renderEncoder->bindRootShaderObject(m_programVars);
        }
        break;
    default:
        throw "unknown pipeline type";
    }
}

SlangResult RenderTestApp::initialize(
    SlangSession* session,
    IDevice* device,
    const Options& options,
    const ShaderCompilerUtil::Input& input)
{
    m_options = options;

    // We begin by compiling the shader file and entry points that specified via the options.
    //
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;

    // Once the shaders have been compiled we load them via the underlying API.
    //
    SLANG_RETURN_ON_FAIL(
        device->createProgram(m_compilationOutput.output.desc, m_shaderProgram.writeRef()));

    // If we are doing a non-pass-through compilation, then we will rely on
    // Slang's reflection API to tell us what the parameters of the program are.
    //
    auto slangReflection = (slang::ProgramLayout*) spGetReflection(m_compilationOutput.output.getRequestForReflection());

    // Once we have determined the layout of all the parameters we need to bind,
    // we will create a shader object to use for storing and binding those parameters.
    //
    m_programVars = device->createRootShaderObject(m_shaderProgram);

    // Now we need to assign from the input parameter data that was parsed into
    // the program vars we allocated.
    //
    SLANG_RETURN_ON_FAIL(_assignVarsFromLayout(
        device, m_programVars, m_compilationOutput.layout, m_outputPlan, slangReflection));

	m_device = device;

    _initializeRenderPass();

    {
        switch(m_options.shaderType)
        {
        default:
            assert(!"unexpected test shader type");
            return SLANG_FAIL;

        case Options::ShaderProgramType::Compute:
            {
                ComputePipelineStateDesc desc;
                desc.program = m_shaderProgram;

                m_pipelineState = device->createComputePipelineState(desc);
            }
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            {
                // TODO: We should conceivably be able to match up the "available" vertex
                // attributes, as defined by the vertex stream(s) on the model being
                // renderer, with the "required" vertex attributes as defiend on the
                // shader.
                //
                // For now we just create a fixed input layout for all graphics tests
                // since at present they all draw the same single triangle with a
                // fixed/known set of attributes.
                //
                const InputElementDesc inputElements[] = {
                    { "A", 0, Format::RGB_Float32, offsetof(Vertex, position) },
                    { "A", 1, Format::RGB_Float32, offsetof(Vertex, color) },
                    { "A", 2, Format::RG_Float32,  offsetof(Vertex, uv) },
                };

                ComPtr<IInputLayout> inputLayout;
                SLANG_RETURN_ON_FAIL(device->createInputLayout(
                    inputElements, SLANG_COUNT_OF(inputElements), inputLayout.writeRef()));

                IBufferResource::Desc vertexBufferDesc;
                vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

                SLANG_RETURN_ON_FAIL(device->createBufferResource(
                    IResource::Usage::VertexBuffer,
                    vertexBufferDesc,
                    kVertexData,
                    m_vertexBuffer.writeRef()));

                GraphicsPipelineStateDesc desc;
                desc.program = m_shaderProgram;
                desc.inputLayout = inputLayout;
                desc.framebufferLayout = m_framebufferLayout;
                m_pipelineState = device->createGraphicsPipelineState(desc);
            }
            break;
        }
    }
    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

Result RenderTestApp::_initializeShaders(
    SlangSession* session,
    IDevice* device,
    Options::ShaderProgramType shaderType,
    const ShaderCompilerUtil::Input& input)
{
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, m_options, input,  m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;
    m_shaderProgram = device->createProgram(m_compilationOutput.output.desc);
    return m_shaderProgram ? SLANG_OK : SLANG_FAIL;
}

void RenderTestApp::_initializeRenderPass()
{
    ITransientResourceHeap::Desc transientHeapDesc = {};
    transientHeapDesc.constantBufferSize = 4096 * 1024;
    m_transientHeap = m_device->createTransientResourceHeap(transientHeapDesc);

    ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
    m_queue = m_device->createCommandQueue(queueDesc);
    
    gfx::ITextureResource::Desc depthBufferDesc;
    depthBufferDesc.setDefaults(gfx::IResource::Usage::DepthWrite);
    depthBufferDesc.init2D(
        gfx::IResource::Type::Texture2D,
        gfx::Format::D_Float32,
        gWindowWidth,
        gWindowHeight,
        0);

    ComPtr<gfx::ITextureResource> depthBufferResource = m_device->createTextureResource(
        gfx::IResource::Usage::DepthWrite, depthBufferDesc, nullptr);

    gfx::ITextureResource::Desc colorBufferDesc;
    colorBufferDesc.setDefaults(gfx::IResource::Usage::RenderTarget);
    colorBufferDesc.init2D(
        gfx::IResource::Type::Texture2D,
        gfx::Format::RGBA_Unorm_UInt8,
        gWindowWidth,
        gWindowHeight,
        0);
    m_colorBuffer = m_device->createTextureResource(
        gfx::IResource::Usage::RenderTarget, colorBufferDesc, nullptr);

    gfx::IResourceView::Desc colorBufferViewDesc;
    memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
    colorBufferViewDesc.format = gfx::Format::RGBA_Unorm_UInt8;
    colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
    colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
    ComPtr<gfx::IResourceView> rtv =
        m_device->createTextureView(m_colorBuffer.get(), colorBufferViewDesc);

    gfx::IResourceView::Desc depthBufferViewDesc;
    memset(&depthBufferViewDesc, 0, sizeof(depthBufferViewDesc));
    depthBufferViewDesc.format = gfx::Format::D_Float32;
    depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
    depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
    ComPtr<gfx::IResourceView> dsv =
        m_device->createTextureView(depthBufferResource.get(), depthBufferViewDesc);

    IFramebufferLayout::AttachmentLayout colorAttachment = {gfx::Format::RGBA_Unorm_UInt8, 1};
    IFramebufferLayout::AttachmentLayout depthAttachment = {gfx::Format::D_Float32, 1};
    gfx::IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &colorAttachment;
    framebufferLayoutDesc.depthStencil = &depthAttachment;
    m_device->createFramebufferLayout(framebufferLayoutDesc, m_framebufferLayout.writeRef());

    gfx::IFramebuffer::Desc framebufferDesc;
    framebufferDesc.renderTargetCount = 1;
    framebufferDesc.depthStencilView = dsv.get();
    framebufferDesc.renderTargetViews = rtv.readRef();
    framebufferDesc.layout = m_framebufferLayout;
    m_device->createFramebuffer(framebufferDesc, m_framebuffer.writeRef());
    
    IRenderPassLayout::Desc renderPassDesc = {};
    renderPassDesc.framebufferLayout = m_framebufferLayout;
    renderPassDesc.renderTargetCount = 1;
    IRenderPassLayout::AttachmentAccessDesc renderTargetAccess = {};
    IRenderPassLayout::AttachmentAccessDesc depthStencilAccess = {};
    renderTargetAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    renderTargetAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
    renderTargetAccess.initialState = ResourceState::Undefined;
    renderTargetAccess.finalState = ResourceState::RenderTarget;
    depthStencilAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    depthStencilAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
    depthStencilAccess.initialState = ResourceState::Undefined;
    depthStencilAccess.finalState = ResourceState::DepthWrite;
    renderPassDesc.renderTargetAccess = &renderTargetAccess;
    renderPassDesc.depthStencilAccess = &depthStencilAccess;
    m_device->createRenderPassLayout(renderPassDesc, m_renderPass.writeRef());
}

void RenderTestApp::setProjectionMatrix(IResourceCommandEncoder* encoder)
{
    SLANG_UNUSED(encoder);
    auto info = m_device->getDeviceInfo();
    ShaderCursor(m_programVars)
        .getField("Uniforms")
        .getDereferenced()
        .setData(info.identityProjectionMatrix, sizeof(float) * 16);
}

void RenderTestApp::renderFrame(IRenderCommandEncoder* encoder)
{
    auto pipelineType = PipelineType::Graphics;

    encoder->setPipelineState(m_pipelineState);

	encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);
    encoder->setVertexBuffer(0, m_vertexBuffer, sizeof(Vertex));

    applyBinding(pipelineType, encoder);

	encoder->draw(3);
}

void RenderTestApp::runCompute(IComputeCommandEncoder* encoder)
{
    auto pipelineType = PipelineType::Compute;
    encoder->setPipelineState(m_pipelineState);
    applyBinding(pipelineType, encoder);
	encoder->dispatchCompute(
        m_options.computeDispatchSize[0],
        m_options.computeDispatchSize[1],
        m_options.computeDispatchSize[2]);
}

void RenderTestApp::finalize()
{
    m_programVars = nullptr;
    m_inputLayout = nullptr;
    m_vertexBuffer = nullptr;
    m_shaderProgram = nullptr;
    m_pipelineState = nullptr;
    m_renderPass = nullptr;
    m_framebuffer = nullptr;
    m_framebufferLayout = nullptr;
    m_colorBuffer = nullptr;
    m_queue = nullptr;
    m_device = nullptr;
}

Result RenderTestApp::writeBindingOutput(const char* fileName)
{
    // Wait until everything is complete
    m_queue->wait();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }
    FileWriter writer(f, WriterFlags(0));

    for(auto outputItem : m_outputPlan.items)
    {
        auto resource = outputItem.resource;
        if (resource && resource->getType() == IResource::Type::Buffer)
        {
            IBufferResource* bufferResource = static_cast<IBufferResource*>(resource.get());
            auto bufferDesc = *bufferResource->getDesc();
            const size_t bufferSize = bufferDesc.sizeInBytes;

            ComPtr<ISlangBlob> blob;
            if(bufferDesc.cpuAccessFlags & IResource::AccessFlag::Read)
            {
                // The buffer is already allocated for CPU access, so we can read it back directly.
                //
                m_device->readBufferResource(bufferResource, 0, bufferSize, blob.writeRef());
            }
            else
            {
                // The buffer is not CPU-readable, so we will copy it using a staging buffer.

                auto stagingBufferDesc = bufferDesc;
                stagingBufferDesc.cpuAccessFlags = IResource::AccessFlag::Read;
                stagingBufferDesc.bindFlags = 0;

                ComPtr<IBufferResource> stagingBuffer;
                SLANG_RETURN_ON_FAIL(m_device->createBufferResource(IResource::Usage::CopyDest, stagingBufferDesc, nullptr, stagingBuffer.writeRef()));

                ComPtr<ICommandBuffer> commandBuffer;
                SLANG_RETURN_ON_FAIL(
                    m_transientHeap->createCommandBuffer(commandBuffer.writeRef()));

                ComPtr<IResourceCommandEncoder> encoder;
                commandBuffer->encodeResourceCommands(encoder.writeRef());
                encoder->copyBuffer(stagingBuffer, 0, bufferResource, 0, bufferSize);
                encoder->endEncoding();

                commandBuffer->close();
                m_queue->executeCommandBuffer(commandBuffer);
                m_transientHeap->synchronizeAndReset();

                m_device->readBufferResource(stagingBuffer, 0, bufferSize, blob.writeRef());
            }

            if (!blob)
            {
                return SLANG_FAIL;
            }
            const SlangResult res = ShaderInputLayout::writeBinding(
                m_options.outputUsingType ? outputItem.typeLayout : nullptr, // TODO: always output using type
                blob->getBufferPointer(),
                bufferSize,
                &writer);
            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            auto typeName = outputItem.typeLayout->getName();
            printf("invalid output type '%s'.\n", typeName ? typeName : "UNKNOWN");
        }
    }
    return SLANG_OK;
}

Result RenderTestApp::writeScreen(const char* filename)
{
    size_t rowPitch, pixelSize;
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(m_device->readTextureResource(
        m_colorBuffer, ResourceState::RenderTarget, blob.writeRef(), &rowPitch, &pixelSize));
    auto bufferSize = blob->getBufferSize();
    uint32_t width = static_cast<uint32_t>(rowPitch / pixelSize);
    uint32_t height = static_cast<uint32_t>(bufferSize / rowPitch);
    return PngSerializeUtil::write(filename, blob, width, height);
}

Result RenderTestApp::update()
{
    auto commandBuffer = m_transientHeap->createCommandBuffer();
    if (m_options.shaderType == Options::ShaderProgramType::Compute)
    {
        auto encoder = commandBuffer->encodeComputeCommands();
        runCompute(encoder);
        encoder->endEncoding();
    }
    else
    {
        auto resEncoder = commandBuffer->encodeResourceCommands();
        setProjectionMatrix(resEncoder);
        resEncoder->endEncoding();

        auto encoder = commandBuffer->encodeRenderCommands(m_renderPass, m_framebuffer);
        gfx::Viewport viewport = {};
        viewport.maxZ = 1.0f;
        viewport.extentX = (float)gWindowWidth;
        viewport.extentY = (float)gWindowHeight;
        encoder->setViewportAndScissor(viewport);
        renderFrame(encoder);
        encoder->endEncoding();
    }
    commandBuffer->close();

    m_startTicks = ProcessUtil::getClockTick();
    m_queue->executeCommandBuffer(commandBuffer);
    m_queue->wait();

    // If we are in a mode where output is requested, we need to snapshot the back buffer here
    if (m_options.outputPath || m_options.performanceProfile)
    {
        // Wait until everything is complete

        if (m_options.performanceProfile)
        {
#if 0
            // It might not be enough on some APIs to 'waitForGpu' to mean the computation has completed. Let's lock an output
            // buffer to be sure
            if (m_bindingState->outputBindings.getCount() > 0)
            {
                const auto& binding = m_bindingState->outputBindings[0];
                auto i = binding.entryIndex;
                const auto& layoutBinding = m_shaderInputLayout.entries[i];

                assert(layoutBinding.isOutput);
                
                if (binding.resource && binding.resource->isBuffer())
                {
                    BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                    const size_t bufferSize = bufferResource->getDesc().sizeInBytes;
                    unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
                    if (!ptr)
                    {                            
                        return SLANG_FAIL;
                    }
                    m_renderer->unmap(bufferResource);
                }
            }
#endif

            // Note we don't do the same with screen rendering -> as that will do a lot of work, which may swamp any computation
            // so can only really profile compute shaders at the moment

            const uint64_t endTicks = ProcessUtil::getClockTick();

            _outputProfileTime(m_startTicks, endTicks);
        }

        if (m_options.outputPath)
        {
            if (m_options.shaderType == Options::ShaderProgramType::Compute || m_options.shaderType == Options::ShaderProgramType::GraphicsCompute)
            {
                auto request = m_compilationOutput.output.getRequestForReflection();
                auto slangReflection = (slang::ShaderReflection*) spGetReflection(request);

                SLANG_RETURN_ON_FAIL(writeBindingOutput(m_options.outputPath));
            }
            else
            {
                SlangResult res = writeScreen(m_options.outputPath);
                if (SLANG_FAILED(res))
                {
                    fprintf(stderr, "ERROR: failed to write screen capture to file\n");
                    return res;
                }
            }
        }
        return SLANG_OK;
    }
    return SLANG_OK;
}


static SlangResult _setSessionPrelude(const Options& options, const char* exePath, SlangSession* session)
{
    // Let's see if we need to set up special prelude for HLSL
    if (options.nvapiExtnSlot.getLength())
    {
        String rootPath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getRootPath(exePath, rootPath));

        String includePath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getIncludePath(rootPath, "external/nvapi/nvHLSLExtns.h", includePath));

        StringBuilder buf;
        // We have to choose a slot that NVAPI will use. 
        buf << "#define NV_SHADER_EXTN_SLOT " << options.nvapiExtnSlot << "\n";

        // Include the NVAPI header
        buf << "#include \"" << includePath << "\"\n\n";

        session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, buf.getBuffer());
    }
    else
    {
        session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, "");
    }

    return SLANG_OK;
}

} //  namespace renderer_test

#if ENABLE_RENDERDOC_INTEGRATION
static RENDERDOC_API_1_1_2* rdoc_api = NULL;
static void initializeRenderDoc()
{
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        assert(ret == 1);
    }
}
static void renderDocBeginFrame() { if (rdoc_api) rdoc_api->StartFrameCapture(nullptr, nullptr); }
static void renderDocEndFrame()
{
    if (rdoc_api)
        rdoc_api->EndFrameCapture(nullptr, nullptr);
    _fgetchar();
}
#else
static void initializeRenderDoc(){}
static void renderDocBeginFrame(){}
static void renderDocEndFrame(){}
#endif

static SlangResult _innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace renderer_test;
    using namespace Slang;

    initializeRenderDoc();

    StdWriters::setSingleton(stdWriters);

    Options options;

	// Parse command-line options
	SLANG_RETURN_ON_FAIL(Options::parse(argcIn, argvIn, StdWriters::getError(), options));

    ShaderCompilerUtil::Input input;
    
    input.profile = "";
    input.target = SLANG_TARGET_NONE;
    input.args = &options.slangArgs[0];
    input.argCount = options.slangArgCount;

	SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
	SlangPassThrough slangPassThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
	switch (options.deviceType)
	{
		case DeviceType::DirectX11:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
			break;

		case DeviceType::DirectX12:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
            if( options.useDXIL )
            {
                input.target = SLANG_DXIL;
                input.profile = "sm_6_0";
                slangPassThrough = SLANG_PASS_THROUGH_DXC;
            }
			break;

		case DeviceType::OpenGl:
			input.target = SLANG_GLSL;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;

		case DeviceType::Vulkan:
			input.target = SLANG_SPIRV;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;
        case DeviceType::CPU:
            input.target = SLANG_HOST_CALLABLE;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            slangPassThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
        case DeviceType::CUDA:
            input.target = SLANG_PTX;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
            slangPassThrough = SLANG_PASS_THROUGH_NVRTC;
            break;

		default:
			fprintf(stderr, "error: unexpected\n");
			return SLANG_FAIL;
	}

    switch (options.inputLanguageID)
    {
        case Options::InputLanguageID::Slang:
            input.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            input.passThrough = SLANG_PASS_THROUGH_NONE;
            break;

        case Options::InputLanguageID::Native:
            input.sourceLanguage = nativeLanguage;
            input.passThrough = slangPassThrough;
            break;

        default:
            break;
    }

    switch( options.shaderType )
    {
    case Options::ShaderProgramType::Graphics:
    case Options::ShaderProgramType::GraphicsCompute:
        input.pipelineType = PipelineType::Graphics;
        break;

    case Options::ShaderProgramType::Compute:
        input.pipelineType = PipelineType::Compute;
        break;

    case Options::ShaderProgramType::RayTracing:
        input.pipelineType = PipelineType::RayTracing;
        break;

    default:
        break;
    }

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_UNKNOWN)
    {
        input.sourceLanguage = options.sourceLanguage;

        if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
        {
            input.passThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
        }
    }

    // Use the profile name set on options if set
    input.profile = options.profileName ? options.profileName : input.profile;

    StringBuilder rendererName;
    auto info = 
    rendererName << "[" << gfxGetDeviceTypeName(options.deviceType) << "] ";
    if (options.adapter.getLength())
    {
        rendererName << "'" << options.adapter << "'";
    }

    if (options.onlyStartup)
    {
        switch (options.deviceType)
        {
            case DeviceType::CUDA:
            {
#if RENDER_TEST_CUDA
                if(SLANG_FAILED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_NVRTC)))
                    return SLANG_FAIL;
#else
                return SLANG_FAIL;
#endif
            }
            case DeviceType::CPU:
            {
                // As long as we have CPU, then this should work
                return spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_GENERIC_C_CPP);
            }
            default: break;
        }
    }

    Index nvapiExtnSlot = -1;

    // Let's see if we need to set up special prelude for HLSL
    if (options.nvapiExtnSlot.getLength() && options.nvapiExtnSlot[0] == 'u')
    {
        //
        Slang::Int value;
        UnownedStringSlice slice = options.nvapiExtnSlot.getUnownedSlice();
        UnownedStringSlice indexText(slice.begin() + 1 , slice.end());
        if (SLANG_SUCCEEDED(StringUtil::parseInt(indexText, value)))
        {
            nvapiExtnSlot = Index(value);
        }
    }

    // If can't set up a necessary prelude make not available (which will lead to the test being ignored)
    if (SLANG_FAILED(_setSessionPrelude(options, argvIn[0], session)))
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    Slang::ComPtr<IDevice> device;
    {
        IDevice::Desc desc = {};
        desc.deviceType = options.deviceType;
        desc.adapter = options.adapter.getBuffer();

        List<const char*> requiredFeatureList;
        for (auto& name : options.renderFeatures)
            requiredFeatureList.add(name.getBuffer());

        desc.requiredFeatures = requiredFeatureList.getBuffer();
        desc.requiredFeatureCount = (int)requiredFeatureList.getCount();

        desc.nvapiExtnSlot = int(nvapiExtnSlot);
        desc.slang.slangGlobalSession = session;

        {
            SlangResult res = gfxCreateDevice(&desc, device.writeRef());
            if (SLANG_FAILED(res))
            {
                // We need to be careful here about SLANG_E_NOT_AVAILABLE. This return value means that the renderer couldn't
                // be created because it required *features* that were *not available*. It does not mean the renderer in general couldn't
                // be constructed.
                //
                // Returning SLANG_E_NOT_AVAILABLE will lead to the test infrastructure ignoring this test.
                //
                // We also don't want to output the 'Unable to create renderer' error, as this isn't an error.
                if (res == SLANG_E_NOT_AVAILABLE)
                {
                    return res;
                }

                if (!options.onlyStartup)
                {
                    fprintf(stderr, "Unable to create renderer %s\n", rendererName.getBuffer());
                }

                return res;
            }
            SLANG_ASSERT(device);
        }

        for (const auto& feature : requiredFeatureList)
        {
            // If doesn't have required feature... we have to give up
            if (!device->hasFeature(feature))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
    }
   
    // If the only test is we can startup, then we are done
    if (options.onlyStartup)
    {
        return SLANG_OK;
    }

	{
        RenderTestApp app;
        renderDocBeginFrame();
        SLANG_RETURN_ON_FAIL(app.initialize(session, device, options, input));
        app.update();
        renderDocEndFrame();
        app.finalize();
        return SLANG_OK;
	}
}

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* sharedSession, int inArgc, const char*const* inArgv)
{
    using namespace Slang;

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded stdlib.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(Index(inArgc - 1), inArgv + 1))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
    }

    SlangResult res = SLANG_FAIL;
    try
    {
        res = _innerMain(stdWriters, session, inArgc, inArgv);
    }
    catch (const Slang::Exception& exception)
    {
        stdWriters->getOut().put(exception.Message.getUnownedSlice());
        return SLANG_FAIL;
    }
    catch (...)
    {
        stdWriters->getOut().put(UnownedStringSlice::fromLiteral("Unhandled exception"));
        return SLANG_FAIL;
    }

    return res;
}

int main(int argc, char**  argv)
{
    using namespace Slang;
    SlangSession* session = spCreateSession(nullptr);

    TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], session);
    
    auto stdWriters = StdWriters::initDefaultSingleton();
    
    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);

	return (int)TestToolUtil::getReturnCode(res);
}

