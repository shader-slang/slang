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
#    include <windows.h>
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
    double time = double(endTicks - startTicks) / Process::getClockFrequency();
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

enum class PipelineType
{
    Graphics,
    Compute,
    RayTracing,
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
    void renderFrameMesh(IRenderCommandEncoder* encoder);
    void finalize();

    Result applyBinding(PipelineType pipelineType, ICommandEncoder* encoder);
    void setProjectionMatrix(IShaderObject* rootObject);
    Result writeBindingOutput(const String& fileName);

    Result writeScreen(const String& filename);

protected:
    /// Called in initialize
    Result _initializeShaders(
        SlangSession* session,
        IDevice* device,
        Options::ShaderProgramType shaderType,
        const ShaderCompilerUtil::Input& input);
    void _initializeRenderPass();
    void _initializeAccelerationStructure();

    uint64_t m_startTicks;

    // variables for state to be used for rendering...
    uintptr_t m_constantBufferSize;

    IDevice* m_device;
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

    ComPtr<IBufferResource> m_blasBuffer;
    ComPtr<IAccelerationStructure> m_bottomLevelAccelerationStructure;
    ComPtr<IBufferResource> m_tlasBuffer;
    ComPtr<IAccelerationStructure> m_topLevelAccelerationStructure;

    ShaderCompilerUtil::OutputAndLayout m_compilationOutput;

    ShaderInputLayout m_shaderInputLayout; ///< The binding layout

    Options m_options;

    ShaderOutputPlan m_outputPlan;
};

struct AssignValsFromLayoutContext
{
    IDevice*                device;
    slang::ISession*        slangSession;
    ShaderOutputPlan&       outputPlan;
    slang::ProgramLayout*   slangReflection;
    IAccelerationStructure* accelerationStructure;

    AssignValsFromLayoutContext(
        IDevice*                    device,
        slang::ISession*            slangSession,
        ShaderOutputPlan&           outputPlan,
        slang::ProgramLayout*       slangReflection,
        IAccelerationStructure*     accelerationStructure)
        : device(device)
        , slangSession(slangSession)
        , outputPlan(outputPlan)
        , slangReflection(slangReflection)
        , accelerationStructure(accelerationStructure)
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

        ComPtr<IBufferResource> counterResource;
        const auto explicitCounterCursor = dstCursor.getExplicitCounter();
        if(srcBuffer.counter != ~0u)
        {
            if(explicitCounterCursor.isValid())
            {
                // If this cursor has a full buffer object associated with the
                // resource, then assign to that.
                ShaderInputLayout::BufferVal counterVal;
                counterVal.bufferData.add(srcBuffer.counter);
                assignBuffer(explicitCounterCursor, &counterVal);
            }
            else
            {
                // Otherwise, this API (D3D) must be handling the buffer object
                // specially, in which case create the buffer resource to pass
                // into `createBufferView`
                const InputBufferDesc& counterBufferDesc{
                    InputBufferType::StorageBuffer,
                    sizeof(uint32_t),
                    Format::Unknown,
                };
                SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBufferResource(
                    counterBufferDesc,
                    sizeof(srcBuffer.counter),
                    &srcBuffer.counter,
                    device,
                    counterResource
                ));
            }
        }
        else if(explicitCounterCursor.isValid())
        {
            // If we know we require a counter for this resource but haven't
            // been given one, error
            return SLANG_E_INVALID_ARG;
        }

        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = srcBuffer.format;
        viewDesc.bufferElementSize = srcVal->bufferDesc.stride;
        auto bufferView = device->createBufferView(bufferResource, counterResource, viewDesc);
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
            textureEntry->textureDesc, ResourceState::ShaderResource, device, texture));

        auto sampler = _createSamplerState(device, samplerEntry->samplerDesc);

        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::ShaderResource;
        auto textureView = device->createTextureView(
            texture,
            viewDesc);

        dstCursor.setCombinedTextureSampler(textureView, sampler);
        maybeAddOutput(dstCursor, srcVal, texture);

        return SLANG_OK;
    }

    SlangResult assignTexture(ShaderCursor const& dstCursor, ShaderInputLayout::TextureVal* srcVal)
    {
        ComPtr<ITextureResource> texture;
        ResourceState defaultState = ResourceState::ShaderResource;
        IResourceView::Type viewType = IResourceView::Type::ShaderResource;

        if (srcVal->textureDesc.isRWTexture)
        {
            defaultState = ResourceState::UnorderedAccess;
            viewType = IResourceView::Type::UnorderedAccess;
        }

        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(
            srcVal->textureDesc, defaultState, device, texture));

        IResourceView::Desc viewDesc = {};
        viewDesc.type = viewType;
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
                // If no name was given, assume by-indexing matching is requested
                auto fieldCursor = dstCursor.getElement((GfxIndex)fieldIndex);
                if(!fieldCursor.isValid())
                {
                    StdWriters::getError().print("error: could not find shader parameter at index %d\n", (int)fieldIndex);
                    return SLANG_E_INVALID_ARG;
                }
                SLANG_RETURN_ON_FAIL(assign(fieldCursor, field.val));
            }
            else
            {
                auto fieldCursor = dstCursor.getPath(field.name.getBuffer());
                if(!fieldCursor.isValid())
                {
                    StdWriters::getError().print("error: could not find shader parameter matching '%s'\n", field.name.begin());
                    return SLANG_E_INVALID_ARG;
                }
                SLANG_RETURN_ON_FAIL(assign(fieldCursor, field.val));
            }
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

        ComPtr<IShaderObject> shaderObject;
        device->createShaderObject2(slangSession, slangType, ShaderObjectContainerType::None, shaderObject.writeRef());

        SLANG_RETURN_ON_FAIL(assign(ShaderCursor(shaderObject), srcVal->contentVal));
        dstCursor.setObject(shaderObject);
        return SLANG_OK;
    }

    SlangResult assignValWithSpecializationArg(
        ShaderCursor const& dstCursor,
        ShaderInputLayout::SpecializeVal* srcVal)
    {
        assign(dstCursor, srcVal->contentVal);
        List<slang::SpecializationArg> args;
        for (auto& typeName : srcVal->typeArgs)
        {
            auto slangType = slangReflection->findTypeByName(typeName.getBuffer());
            if (!slangType)
            {
                StdWriters::getError().print("error: could not find shader type '%s'\n", typeName.getBuffer());
                return SLANG_E_INVALID_ARG;
            }
            args.add(slang::SpecializationArg::fromType(slangType));
        }
        return dstCursor.setSpecializationArgs(args.getBuffer(), (uint32_t)args.getCount());
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

    SlangResult assignAccelerationStructure(
        ShaderCursor const& dstCursor,
        ShaderInputLayout::AccelerationStructureVal* srcVal)
    {
        dstCursor.setResource(accelerationStructure);
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

        case ShaderInputType::Specialize:
            return assignValWithSpecializationArg(
                dstCursor, (ShaderInputLayout::SpecializeVal*)srcVal.Ptr());

        case ShaderInputType::Aggregate:
            return assignAggregate(dstCursor, (ShaderInputLayout::AggVal*) srcVal.Ptr());

        case ShaderInputType::Array:
            return assignArray(dstCursor, (ShaderInputLayout::ArrayVal*) srcVal.Ptr());

        case ShaderInputType::AccelerationStructure:
            return assignAccelerationStructure(
                dstCursor, (ShaderInputLayout::AccelerationStructureVal*)srcVal.Ptr());
        default:
            assert(!"Unhandled type");
            return SLANG_FAIL;
        }
    }
};

SlangResult _assignVarsFromLayout(
    IDevice*                    device,
    slang::ISession*            slangSession,
    IShaderObject*              shaderObject,
    ShaderInputLayout const&    layout,
    ShaderOutputPlan&           ioOutputPlan,
    slang::ProgramLayout*       slangReflection,
    IAccelerationStructure*     accelerationStructure)
{
    AssignValsFromLayoutContext context(
        device, slangSession, ioOutputPlan, slangReflection, accelerationStructure);
    ShaderCursor rootCursor = ShaderCursor(shaderObject);
    return context.assign(rootCursor, layout.rootVal);
}

Result RenderTestApp::applyBinding(PipelineType pipelineType, ICommandEncoder* encoder)
{
    auto slangReflection = (slang::ProgramLayout*)spGetReflection(
        m_compilationOutput.output.getRequestForReflection());
    ComPtr<slang::ISession> slangSession;
    m_compilationOutput.output.m_requestForKernels->getSession(slangSession.writeRef());

    switch (pipelineType)
    {
    case PipelineType::Compute:
        {
            IComputeCommandEncoder* computeEncoder = static_cast<IComputeCommandEncoder*>(encoder);
            auto rootObject = computeEncoder->bindPipeline(m_pipelineState);
            SLANG_RETURN_ON_FAIL(_assignVarsFromLayout(
                m_device,
                slangSession,
                rootObject,
                m_compilationOutput.layout,
                m_outputPlan,
                slangReflection,
                m_topLevelAccelerationStructure));
        }
        break;
    case PipelineType::Graphics:
        {
            IRenderCommandEncoder* renderEncoder = static_cast<IRenderCommandEncoder*>(encoder);
            auto rootObject = renderEncoder->bindPipeline(m_pipelineState);
            SLANG_RETURN_ON_FAIL(_assignVarsFromLayout(
                m_device,
                slangSession,
                rootObject,
                m_compilationOutput.layout,
                m_outputPlan,
                slangReflection,
                m_topLevelAccelerationStructure));
            setProjectionMatrix(rootObject);
        }
        break;
    default:
        throw "unknown pipeline type";
    }
    return SLANG_OK;
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
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(device->getSlangSession()->getGlobalSession(), options, input, m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;

    // Once the shaders have been compiled we load them via the underlying API.
    //
    ComPtr<ISlangBlob> outDiagnostics;
    auto result = device->createProgram(m_compilationOutput.output.desc, m_shaderProgram.writeRef(), outDiagnostics.writeRef());

    // If there was a failure creating a program, we can't continue
    // Special case SLANG_E_NOT_AVAILABLE error code to make it a failure,
    // as it is also used to indicate an attempt setup something failed gracefully (because it couldn't be supported)
    // but that's not this.
    if (SLANG_FAILED(result))
    {
        result = (result == SLANG_E_NOT_AVAILABLE) ? SLANG_FAIL : result;
        return result;
    }

	m_device = device;

    _initializeRenderPass();
    _initializeAccelerationStructure();

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
                    { "A", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position) },
                    { "A", 1, Format::R32G32B32_FLOAT, offsetof(Vertex, color) },
                    { "A", 2, Format::R32G32_FLOAT,  offsetof(Vertex, uv) },
                };

                ComPtr<IInputLayout> inputLayout;
                SLANG_RETURN_ON_FAIL(device->createInputLayout(
                    sizeof(Vertex), inputElements, SLANG_COUNT_OF(inputElements), inputLayout.writeRef()));

                IBufferResource::Desc vertexBufferDesc;
                vertexBufferDesc.type = IResource::Type::Buffer;
                vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
                vertexBufferDesc.memoryType = MemoryType::Upload;
                vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
                vertexBufferDesc.allowedStates = ResourceStateSet(ResourceState::VertexBuffer);

                SLANG_RETURN_ON_FAIL(device->createBufferResource(
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
        case Options::ShaderProgramType::GraphicsMeshCompute:
        case Options::ShaderProgramType::GraphicsTaskMeshCompute:
            {
                GraphicsPipelineStateDesc desc;
                desc.program = m_shaderProgram;
                desc.framebufferLayout = m_framebufferLayout;
                m_pipelineState = device->createGraphicsPipelineState(desc);
            }
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
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(device->getSlangSession()->getGlobalSession(), m_options, input, m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;
    m_shaderProgram = device->createProgram(m_compilationOutput.output.desc);
    return m_shaderProgram ? SLANG_OK : SLANG_FAIL;
}

void RenderTestApp::_initializeRenderPass()
{
    ITransientResourceHeap::Desc transientHeapDesc = {};
    transientHeapDesc.constantBufferSize = 4096 * 1024;
    m_transientHeap = m_device->createTransientResourceHeap(transientHeapDesc);
    SLANG_ASSERT(m_transientHeap);

    ICommandQueue::Desc queueDesc = {ICommandQueue::QueueType::Graphics};
    m_queue = m_device->createCommandQueue(queueDesc);
    SLANG_ASSERT(m_queue);
    
    gfx::ITextureResource::Desc depthBufferDesc;
    depthBufferDesc.type = IResource::Type::Texture2D;
    depthBufferDesc.size.width = gWindowWidth;
    depthBufferDesc.size.height = gWindowHeight;
    depthBufferDesc.size.depth = 1;
    depthBufferDesc.numMipLevels = 1;
    depthBufferDesc.format = Format::D32_FLOAT;
    depthBufferDesc.defaultState = ResourceState::DepthWrite;
    depthBufferDesc.allowedStates = ResourceState::DepthWrite;

    ComPtr<gfx::ITextureResource> depthBufferResource =
        m_device->createTextureResource(depthBufferDesc, nullptr);
    SLANG_ASSERT(depthBufferResource);

    gfx::ITextureResource::Desc colorBufferDesc;
    colorBufferDesc.type = IResource::Type::Texture2D;
    colorBufferDesc.size.width = gWindowWidth;
    colorBufferDesc.size.height = gWindowHeight;
    colorBufferDesc.size.depth = 1;
    colorBufferDesc.numMipLevels = 1;
    colorBufferDesc.format = Format::R8G8B8A8_UNORM;
    colorBufferDesc.defaultState = ResourceState::RenderTarget;
    colorBufferDesc.allowedStates = ResourceState::RenderTarget;
    m_colorBuffer = m_device->createTextureResource(colorBufferDesc, nullptr);
    SLANG_ASSERT(m_colorBuffer);

    gfx::IResourceView::Desc colorBufferViewDesc = {};
    memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
    colorBufferViewDesc.format = gfx::Format::R8G8B8A8_UNORM;
    colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
    colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
    ComPtr<gfx::IResourceView> rtv =
        m_device->createTextureView(m_colorBuffer.get(), colorBufferViewDesc);
    SLANG_ASSERT(rtv);

    gfx::IResourceView::Desc depthBufferViewDesc = {};
    memset(&depthBufferViewDesc, 0, sizeof(depthBufferViewDesc));
    depthBufferViewDesc.format = gfx::Format::D32_FLOAT;
    depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
    depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
    ComPtr<gfx::IResourceView> dsv =
        m_device->createTextureView(depthBufferResource.get(), depthBufferViewDesc);
    SLANG_ASSERT(dsv);

    IFramebufferLayout::TargetLayout colorTarget = {gfx::Format::R8G8B8A8_UNORM, 1};
    IFramebufferLayout::TargetLayout depthTarget = {gfx::Format::D32_FLOAT, 1};
    gfx::IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &colorTarget;
    framebufferLayoutDesc.depthStencil = &depthTarget;
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
    IRenderPassLayout::TargetAccessDesc renderTargetAccess = {};
    IRenderPassLayout::TargetAccessDesc depthStencilAccess = {};
    renderTargetAccess.loadOp = IRenderPassLayout::TargetLoadOp::Clear;
    renderTargetAccess.storeOp = IRenderPassLayout::TargetStoreOp::Store;
    renderTargetAccess.initialState = ResourceState::Undefined;
    renderTargetAccess.finalState = ResourceState::RenderTarget;
    depthStencilAccess.loadOp = IRenderPassLayout::TargetLoadOp::Clear;
    depthStencilAccess.storeOp = IRenderPassLayout::TargetStoreOp::Store;
    depthStencilAccess.initialState = ResourceState::Undefined;
    depthStencilAccess.finalState = ResourceState::DepthWrite;
    renderPassDesc.renderTargetAccess = &renderTargetAccess;
    renderPassDesc.depthStencilAccess = &depthStencilAccess;
    m_device->createRenderPassLayout(renderPassDesc, m_renderPass.writeRef());
}

void RenderTestApp::_initializeAccelerationStructure()
{
    if (!m_device->hasFeature("ray-tracing"))
        return;
    IBufferResource::Desc vertexBufferDesc = {};
    vertexBufferDesc.type = IResource::Type::Buffer;
    vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
    vertexBufferDesc.defaultState = ResourceState::ShaderResource;
    ComPtr<IBufferResource> vertexBuffer =
        m_device->createBufferResource(vertexBufferDesc, &kVertexData[0]);

    IBufferResource::Desc transformBufferDesc = {};
    transformBufferDesc.type = IResource::Type::Buffer;
    transformBufferDesc.sizeInBytes = sizeof(float) * 12;
    transformBufferDesc.defaultState = ResourceState::ShaderResource;
    float transformData[12] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    ComPtr<IBufferResource> transformBuffer =
        m_device->createBufferResource(transformBufferDesc, &transformData);

    // Build bottom level acceleration structure.
    {
        IAccelerationStructure::BuildInputs accelerationStructureBuildInputs = {};
        IAccelerationStructure::PrebuildInfo accelerationStructurePrebuildInfo = {};
        accelerationStructureBuildInputs.descCount = 1;
        accelerationStructureBuildInputs.kind = IAccelerationStructure::Kind::BottomLevel;
        accelerationStructureBuildInputs.flags =
            IAccelerationStructure::BuildFlags::AllowCompaction;
        IAccelerationStructure::GeometryDesc geomDesc = {};
        geomDesc.flags = IAccelerationStructure::GeometryFlags::Opaque;
        geomDesc.type = IAccelerationStructure::GeometryType::Triangles;
        geomDesc.content.triangles.indexCount = 0;
        geomDesc.content.triangles.indexData = 0;
        geomDesc.content.triangles.indexFormat = Format::Unknown;
        geomDesc.content.triangles.vertexCount = kVertexCount;
        geomDesc.content.triangles.vertexData = vertexBuffer->getDeviceAddress();
        geomDesc.content.triangles.vertexFormat = Format::R32G32B32_FLOAT;
        geomDesc.content.triangles.vertexStride = sizeof(Vertex);
        geomDesc.content.triangles.transform3x4 = transformBuffer->getDeviceAddress();
        accelerationStructureBuildInputs.geometryDescs = &geomDesc;

        // Query buffer size for acceleration structure build.
        m_device->getAccelerationStructurePrebuildInfo(
            accelerationStructureBuildInputs, &accelerationStructurePrebuildInfo);
        // Allocate buffers for acceleration structure.
        IBufferResource::Desc asDraftBufferDesc = {};
        asDraftBufferDesc.type = IResource::Type::Buffer;
        asDraftBufferDesc.defaultState = ResourceState::AccelerationStructure;
        asDraftBufferDesc.sizeInBytes = accelerationStructurePrebuildInfo.resultDataMaxSize;
        ComPtr<IBufferResource> draftBuffer = m_device->createBufferResource(asDraftBufferDesc);
        IBufferResource::Desc scratchBufferDesc = {};
        scratchBufferDesc.type = IResource::Type::Buffer;
        scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
        scratchBufferDesc.sizeInBytes = accelerationStructurePrebuildInfo.scratchDataSize;
        ComPtr<IBufferResource> scratchBuffer = m_device->createBufferResource(scratchBufferDesc);

        // Build acceleration structure.
        ComPtr<IQueryPool> compactedSizeQuery;
        IQueryPool::Desc queryPoolDesc = {};
        queryPoolDesc.count = 1;
        queryPoolDesc.type = QueryType::AccelerationStructureCompactedSize;
        m_device->createQueryPool(queryPoolDesc, compactedSizeQuery.writeRef());

        ComPtr<IAccelerationStructure> draftAS;
        IAccelerationStructure::CreateDesc draftCreateDesc = {};
        draftCreateDesc.buffer = draftBuffer;
        draftCreateDesc.kind = IAccelerationStructure::Kind::BottomLevel;
        draftCreateDesc.offset = 0;
        draftCreateDesc.size = accelerationStructurePrebuildInfo.resultDataMaxSize;
        m_device->createAccelerationStructure(draftCreateDesc, draftAS.writeRef());

        compactedSizeQuery->reset();

        auto commandBuffer = m_transientHeap->createCommandBuffer();
        auto encoder = commandBuffer->encodeRayTracingCommands();
        IAccelerationStructure::BuildDesc buildDesc = {};
        buildDesc.dest = draftAS;
        buildDesc.inputs = accelerationStructureBuildInputs;
        buildDesc.scratchData = scratchBuffer->getDeviceAddress();
        AccelerationStructureQueryDesc compactedSizeQueryDesc = {};
        compactedSizeQueryDesc.queryPool = compactedSizeQuery;
        compactedSizeQueryDesc.queryType = QueryType::AccelerationStructureCompactedSize;
        encoder->buildAccelerationStructure(buildDesc, 1, &compactedSizeQueryDesc);
        encoder->endEncoding();
        commandBuffer->close();
        m_queue->executeCommandBuffer(commandBuffer);
        m_queue->waitOnHost();

        uint64_t compactedSize = 0;
        compactedSizeQuery->getResult(0, 1, &compactedSize);
        IBufferResource::Desc asBufferDesc = {};
        asBufferDesc.type = IResource::Type::Buffer;
        asBufferDesc.defaultState = ResourceState::AccelerationStructure;
        asBufferDesc.sizeInBytes = (Size)compactedSize;
        m_blasBuffer = m_device->createBufferResource(asBufferDesc);
        IAccelerationStructure::CreateDesc createDesc;
        createDesc.buffer = m_blasBuffer;
        createDesc.kind = IAccelerationStructure::Kind::BottomLevel;
        createDesc.offset = 0;
        createDesc.size = (Size)compactedSize;
        m_device->createAccelerationStructure(createDesc, m_bottomLevelAccelerationStructure.writeRef());

        commandBuffer = m_transientHeap->createCommandBuffer();
        encoder = commandBuffer->encodeRayTracingCommands();
        encoder->copyAccelerationStructure(
            m_bottomLevelAccelerationStructure, draftAS, AccelerationStructureCopyMode::Compact);
        encoder->endEncoding();
        commandBuffer->close();
        m_queue->executeCommandBuffer(commandBuffer);
        m_queue->waitOnHost();
    }

    // Build top level acceleration structure.
    {
        List<IAccelerationStructure::InstanceDesc> instanceDescs;
        instanceDescs.setCount(1);
        instanceDescs[0].accelerationStructure =
            m_bottomLevelAccelerationStructure->getDeviceAddress();
        instanceDescs[0].flags =
            IAccelerationStructure::GeometryInstanceFlags::TriangleFacingCullDisable;
        instanceDescs[0].instanceContributionToHitGroupIndex = 0;
        instanceDescs[0].instanceID = 0;
        instanceDescs[0].instanceMask = 0xFF;
        float transformMatrix[] = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        memcpy(&instanceDescs[0].transform[0][0], transformMatrix, sizeof(float) * 12);

        IBufferResource::Desc instanceBufferDesc = {};
        instanceBufferDesc.type = IResource::Type::Buffer;
        instanceBufferDesc.sizeInBytes =
            instanceDescs.getCount() * sizeof(IAccelerationStructure::InstanceDesc);
        instanceBufferDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        ComPtr<IBufferResource> instanceBuffer =
            m_device->createBufferResource(instanceBufferDesc, instanceDescs.getBuffer());

        IAccelerationStructure::BuildInputs accelerationStructureBuildInputs = {};
        IAccelerationStructure::PrebuildInfo accelerationStructurePrebuildInfo = {};
        accelerationStructureBuildInputs.descCount = 1;
        accelerationStructureBuildInputs.kind = IAccelerationStructure::Kind::TopLevel;
        accelerationStructureBuildInputs.instanceDescs = instanceBuffer->getDeviceAddress();

        // Query buffer size for acceleration structure build.
        m_device->getAccelerationStructurePrebuildInfo(
            accelerationStructureBuildInputs, &accelerationStructurePrebuildInfo);

        IBufferResource::Desc asBufferDesc = {};
        asBufferDesc.type = IResource::Type::Buffer;
        asBufferDesc.defaultState = ResourceState::AccelerationStructure;
        asBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
        m_tlasBuffer = m_device->createBufferResource(asBufferDesc);

        IBufferResource::Desc scratchBufferDesc = {};
        scratchBufferDesc.type = IResource::Type::Buffer;
        scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
        scratchBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.scratchDataSize;
        ComPtr<IBufferResource> scratchBuffer = m_device->createBufferResource(scratchBufferDesc);

        IAccelerationStructure::CreateDesc createDesc = {};
        createDesc.buffer = m_tlasBuffer;
        createDesc.kind = IAccelerationStructure::Kind::TopLevel;
        createDesc.offset = 0;
        createDesc.size = accelerationStructurePrebuildInfo.resultDataMaxSize;
        m_device->createAccelerationStructure(
            createDesc, m_topLevelAccelerationStructure.writeRef());

        auto commandBuffer = m_transientHeap->createCommandBuffer();
        auto encoder = commandBuffer->encodeRayTracingCommands();
        IAccelerationStructure::BuildDesc buildDesc = {};
        buildDesc.dest = m_topLevelAccelerationStructure;
        buildDesc.inputs = accelerationStructureBuildInputs;
        buildDesc.scratchData = scratchBuffer->getDeviceAddress();
        encoder->buildAccelerationStructure(buildDesc, 0, nullptr);
        encoder->endEncoding();
        commandBuffer->close();
        m_queue->executeCommandBuffer(commandBuffer);
        m_queue->waitOnHost();
    }
}

void RenderTestApp::setProjectionMatrix(IShaderObject* rootObject)
{
    auto info = m_device->getDeviceInfo();
    ShaderCursor(rootObject)
        .getField("Uniforms")
        .getDereferenced()
        .setData(info.identityProjectionMatrix, sizeof(float) * 16);
}

void RenderTestApp::renderFrameMesh(IRenderCommandEncoder* encoder)
{
    auto pipelineType = PipelineType::Graphics;
    applyBinding(pipelineType, encoder);
	encoder->drawMeshTasks(
        m_options.computeDispatchSize[0],
        m_options.computeDispatchSize[1],
        m_options.computeDispatchSize[2]
    );
}

void RenderTestApp::renderFrame(IRenderCommandEncoder* encoder)
{
    auto pipelineType = PipelineType::Graphics;
    applyBinding(pipelineType, encoder);

	encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);
    encoder->setVertexBuffer(0, m_vertexBuffer);

	encoder->draw(3);
}

void RenderTestApp::runCompute(IComputeCommandEncoder* encoder)
{
    auto pipelineType = PipelineType::Compute;
    applyBinding(pipelineType, encoder);
	encoder->dispatchCompute(
        m_options.computeDispatchSize[0],
        m_options.computeDispatchSize[1],
        m_options.computeDispatchSize[2]);
}

void RenderTestApp::finalize()
{
    m_compilationOutput.output.reset();
}

Result RenderTestApp::writeBindingOutput(const String& fileName)
{
    // Wait until everything is complete
    m_queue->waitOnHost();

    FILE * f = fopen(fileName.getBuffer(), "wb");
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
            if(bufferDesc.memoryType == MemoryType::ReadBack)
            {
                // The buffer is already allocated for CPU access, so we can read it back directly.
                //
                m_device->readBufferResource(bufferResource, 0, bufferSize, blob.writeRef());
            }
            else
            {
                // The buffer is not CPU-readable, so we will copy it using a staging buffer.

                auto stagingBufferDesc = bufferDesc;
                stagingBufferDesc.memoryType = MemoryType::ReadBack;
                stagingBufferDesc.allowedStates =
                    ResourceStateSet(ResourceState::CopyDestination, ResourceState::CopySource);
                stagingBufferDesc.defaultState = ResourceState::CopyDestination;

                ComPtr<IBufferResource> stagingBuffer;
                SLANG_RETURN_ON_FAIL(m_device->createBufferResource(stagingBufferDesc, nullptr, stagingBuffer.writeRef()));

                ComPtr<ICommandBuffer> commandBuffer;
                SLANG_RETURN_ON_FAIL(
                    m_transientHeap->createCommandBuffer(commandBuffer.writeRef()));

                IResourceCommandEncoder* encoder = nullptr;
                commandBuffer->encodeResourceCommands(&encoder);
                encoder->copyBuffer(stagingBuffer, 0, bufferResource, 0, bufferSize);
                encoder->endEncoding();

                commandBuffer->close();
                m_queue->executeCommandBuffer(commandBuffer);
                m_transientHeap->finish();
                m_transientHeap->synchronizeAndReset();

                SLANG_RETURN_ON_FAIL(m_device->readBufferResource(stagingBuffer, 0, bufferSize, blob.writeRef()));
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

Result RenderTestApp::writeScreen(const String& filename)
{
    size_t rowPitch, pixelSize;
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(m_device->readTextureResource(
        m_colorBuffer, ResourceState::RenderTarget, blob.writeRef(), &rowPitch, &pixelSize));
    auto bufferSize = blob->getBufferSize();
    uint32_t width = static_cast<uint32_t>(rowPitch / pixelSize);
    uint32_t height = static_cast<uint32_t>(bufferSize / rowPitch);
    return PngSerializeUtil::write(filename.getBuffer(), blob, width, height);
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
        auto encoder = commandBuffer->encodeRenderCommands(m_renderPass, m_framebuffer);
        gfx::Viewport viewport = {};
        viewport.maxZ = 1.0f;
        viewport.extentX = (float)gWindowWidth;
        viewport.extentY = (float)gWindowHeight;
        encoder->setViewportAndScissor(viewport);
        if(m_options.shaderType == Options::ShaderProgramType::GraphicsMeshCompute
            || m_options.shaderType == Options::ShaderProgramType::GraphicsTaskMeshCompute)
            renderFrameMesh(encoder);
        else
            renderFrame(encoder);
        encoder->endEncoding();
    }
    commandBuffer->close();

    m_startTicks = Process::getClockTick();
    m_queue->executeCommandBuffer(commandBuffer);
    m_queue->waitOnHost();

    // If we are in a mode where output is requested, we need to snapshot the back buffer here
    if (m_options.outputPath.getLength() || m_options.performanceProfile)
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

            const uint64_t endTicks = Process::getClockTick();

            _outputProfileTime(m_startTicks, endTicks);
        }

        if (m_options.outputPath.getLength())
        {
            if (m_options.shaderType == Options::ShaderProgramType::Compute
                || m_options.shaderType == Options::ShaderProgramType::GraphicsCompute
                || m_options.shaderType == Options::ShaderProgramType::GraphicsMeshCompute
                || m_options.shaderType == Options::ShaderProgramType::GraphicsTaskMeshCompute)
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
#if !SLANG_WINDOWS_FAMILY
        // NVAPI is currently only available on Windows
        return SLANG_E_NOT_AVAILABLE;
#else
        // We want to set the path to NVAPI
        String rootPath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getRootPath(exePath, rootPath));
        String includePath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getIncludePath(rootPath, "external/nvapi/nvHLSLExtns.h", includePath))

        StringBuilder buf;
        // We have to choose a slot that NVAPI will use. 
        buf << "#define NV_SHADER_EXTN_SLOT " << options.nvapiExtnSlot << "\n";

        // Include the NVAPI header
        buf << "#include ";
        StringEscapeUtil::appendQuoted(StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp), includePath.getUnownedSlice(), buf);
        buf << "\n\n";

        session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, buf.getBuffer());
#endif
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

class StdWritersDebugCallback : public gfx::IDebugCallback
{
public:
    Slang::StdWriters* writers;
    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        gfx::DebugMessageType type,
        gfx::DebugMessageSource source,
        const char* message) override
    {
        SLANG_UNUSED(source);
        if (type == gfx::DebugMessageType::Error)
        {
            writers->getOut().print("%s\n", message);
        }
    }
};

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
            input.target = SLANG_SHADER_HOST_CALLABLE;
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

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_UNKNOWN)
    {
        input.sourceLanguage = options.sourceLanguage;

        if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
        {
            input.passThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
        }
    }

#ifdef _DEBUG
    gfxEnableDebugLayer();
#endif
    StdWritersDebugCallback debugCallback;
    debugCallback.writers = stdWriters;
    gfxSetDebugCallback(&debugCallback);
    struct ResetDebugCallbackRAII
    {
        ~ResetDebugCallbackRAII()
        {
            gfxSetDebugCallback(nullptr);
        }
    } resetDebugCallbackRAII;

    // Use the profile name set on options if set
    input.profile = options.profileName.getLength() ? options.profileName : input.profile;

    StringBuilder rendererName;
    auto info = 
    rendererName << "[" << gfxGetDeviceTypeName(options.deviceType) << "] ";

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

        desc.slang.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_NONE;
        if (options.generateSPIRVDirectly)
            desc.slang.targetFlags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

        List<const char*> requiredFeatureList;
        for (auto& name : options.renderFeatures)
            requiredFeatureList.add(name.getBuffer());

        desc.requiredFeatures = requiredFeatureList.getBuffer();
        desc.requiredFeatureCount = (int)requiredFeatureList.getCount();

        // Look for args going to slang
        {
            const auto& args = options.downstreamArgs.getArgsByName("slang");
            for (const auto& arg : args)
            {
                if (arg.value == "-matrix-layout-column-major")
                {
                    desc.slang.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
                    break;
                }
            }
        }
        
        desc.nvapiExtnSlot = int(nvapiExtnSlot);
        desc.slang.slangGlobalSession = session;
        desc.slang.targetProfile = options.profileName.getBuffer();
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
	}
    return SLANG_OK;
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

