// render-test-main.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "options.h"
#include "render.h"
#include "tools/gfx-util/shader-cursor.h"
#include "slang-support.h"
#include "surface.h"
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

#include "cpu-compute-util.h"

#if RENDER_TEST_CUDA
#   include "cuda/cuda-compute-util.h"
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
        Index               inputLayoutEntryIndex;
        ComPtr<IResource>   resource;
    };

    List<Item> items;
};

class RenderTestApp : public WindowListener
{
public:
    // WindowListener
    virtual Result update(Window* window) SLANG_OVERRIDE;

    // At initialization time, we are going to load and compile our Slang shader
    // code, and then create the API objects we need for rendering.
    virtual Result initialize(
        SlangSession* session,
        IRenderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) = 0;
    void runCompute();
    void renderFrame();
    void finalize();

    virtual void applyBinding(PipelineType pipelineType) = 0;
    virtual void setProjectionMatrix() = 0;
    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) = 0;

    Result writeScreen(const char* filename);

protected:
    /// Called in initialize
    Result _initializeShaders(
        SlangSession* session,
        IRenderer* renderer,
        Options::ShaderProgramType shaderType,
        const ShaderCompilerUtil::Input& input);

    virtual void finalizeImpl();

    uint64_t m_startTicks;

    // variables for state to be used for rendering...
    uintptr_t m_constantBufferSize;

    ComPtr<IRenderer> m_renderer;

    ComPtr<IInputLayout> m_inputLayout;
    ComPtr<IBufferResource> m_vertexBuffer;
    ComPtr<IShaderProgram> m_shaderProgram;
    ComPtr<IPipelineState> m_pipelineState;

    ShaderCompilerUtil::OutputAndLayout m_compilationOutput;

    ShaderInputLayout m_shaderInputLayout; ///< The binding layout

    Options m_options;
};

class LegacyRenderTestApp : public RenderTestApp
{
public:
    virtual void applyBinding(PipelineType pipelineType) SLANG_OVERRIDE;
    virtual void setProjectionMatrix() SLANG_OVERRIDE;
    virtual Result initialize(
        SlangSession* session,
        IRenderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) SLANG_OVERRIDE;

    BindingStateImpl* getBindingState() const { return m_bindingState; }

    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) override;

protected:
	uintptr_t m_constantBufferSize;
	ComPtr<IBufferResource>	m_constantBuffer;
	RefPtr<BindingStateImpl>    m_bindingState;
    int m_numAddedConstantBuffers;                      ///< Constant buffers can be added to the binding directly. Will be added at the end.
};

class ShaderObjectRenderTestApp : public RenderTestApp
{
public:
    virtual void applyBinding(PipelineType pipelineType) SLANG_OVERRIDE;
    virtual void setProjectionMatrix() SLANG_OVERRIDE;
    virtual Result initialize(
        SlangSession* session,
        IRenderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) SLANG_OVERRIDE;
    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) override;

protected:
    virtual void finalizeImpl() SLANG_OVERRIDE;

    ComPtr<IShaderObject> m_programVars;
    ShaderOutputPlan m_outputPlan;
};

SlangResult _assignVarsFromLayout(
    IRenderer*                   renderer,
    IShaderObject*               shaderObject,
    ShaderInputLayout const&    layout,
    ShaderOutputPlan&           ioOutputPlan,
    slang::ProgramLayout*       slangReflection)
{
    ShaderCursor rootCursor = ShaderCursor(shaderObject);

    const int textureBindFlags = IResource::BindFlag::NonPixelShaderResource | IResource::BindFlag::PixelShaderResource;

    Index entryCount = layout.entries.getCount();
    for(Index entryIndex = 0; entryIndex < entryCount; ++entryIndex)
    {
        auto& entry = layout.entries[entryIndex];
        if(entry.name.getLength() == 0)
        {
            StdWriters::getError().print("error: entries in `ShaderInputLayout` must include a name\n");
            return SLANG_E_INVALID_ARG;
        }

        auto entryCursor = rootCursor.getPath(entry.name.getBuffer());

        if(!entryCursor.isValid())
        {
            for(gfx::UInt i = 0; i < shaderObject->getEntryPointCount(); i++)
            {
                entryCursor = ShaderCursor(shaderObject->getEntryPoint(i)).getPath(entry.name.getBuffer());
                if(entryCursor.isValid())
                    break;
            }
        }


        if(!entryCursor.isValid())
        {
            StdWriters::getError().print("error: could not find shader parameter matching '%s'\n", entry.name.begin());
            return SLANG_E_INVALID_ARG;
        }

        ComPtr<IResource> resource;
        switch(entry.type)
        {
        case ShaderInputType::Uniform:
            {
                const size_t bufferSize = entry.bufferData.getCount() * sizeof(uint32_t);

                ShaderCursor dataCursor = entryCursor;
                switch(dataCursor.getTypeLayout()->getKind())
                {
                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                    dataCursor = dataCursor.getDereferenced();
                    break;

                default:
                    break;

                }

                dataCursor.setData(entry.bufferData.getBuffer(), bufferSize);
            }
            break;

        case ShaderInputType::Buffer:
            {
                const InputBufferDesc& srcBuffer = entry.bufferDesc;
                const size_t bufferSize = entry.bufferData.getCount() * sizeof(uint32_t);

                switch(srcBuffer.type)
                {
                case InputBufferType::ConstantBuffer:
                    {
                        // A `cbuffer` input line actually represents the data we
                        // want to write *into* the buffer, and shouldn't
                        // allocate a buffer itself.
                        //
                        entryCursor.getDereferenced().setData(entry.bufferData.getBuffer(), bufferSize);
                    }
                    break;

                case InputBufferType::RootConstantBuffer:
                    {
                        // A `root_constants` input line actually represents the data we
                        // want to write *into* the buffer, and shouldn't
                        // allocate a buffer itself.
                        //
                        // Note: we are not doing `.getDereferenced()` here because the
                        // `root_constant` line should be referring to a parameter value
                        // inside the root-constant range, and not the range/buffer itself.
                        //
                        entryCursor.setData(entry.bufferData.getBuffer(), bufferSize);
                    }
                    break;

                default:
                    {

                        ComPtr<IBufferResource> bufferResource;
                        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBufferResource(entry.bufferDesc, entry.isOutput, bufferSize, entry.bufferData.getBuffer(), renderer, bufferResource));
                        resource = bufferResource;

                        IResourceView::Desc viewDesc;
                        viewDesc.type = IResourceView::Type::UnorderedAccess;
                        viewDesc.format = srcBuffer.format;
                        auto bufferView = renderer->createBufferView(
                            bufferResource,
                            viewDesc);
                        entryCursor.setResource(bufferView);
                    }
                    break;
                }

#if 0
                switch(srcBuffer.type)
                {
                case InputBufferType::ConstantBuffer:
                    descriptorSet->setConstantBuffer(rangeIndex, 0, bufferResource);
                    break;

                case InputBufferType::StorageBuffer:
                    {
                        ResourceView::Desc viewDesc;
                        viewDesc.type = ResourceView::Type::UnorderedAccess;
                        viewDesc.format = srcBuffer.format;
                        auto bufferView = renderer->createBufferView(
                            bufferResource,
                            viewDesc);
                        descriptorSet->setResource(rangeIndex, 0, bufferView);
                    }
                    break;
                }

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = bufferResource;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::CombinedTextureSampler:
            {
                ComPtr<ITextureResource> texture;
                SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(entry.textureDesc, textureBindFlags, renderer, texture));
                resource = texture;

                auto sampler = _createSamplerState(renderer, entry.samplerDesc);

                IResourceView::Desc viewDesc;
                viewDesc.type = IResourceView::Type::ShaderResource;
                auto textureView = renderer->createTextureView(
                    texture,
                    viewDesc);

                entryCursor.setCombinedTextureSampler(textureView, sampler);

#if 0
                descriptorSet->setCombinedTextureSampler(rangeIndex, 0, textureView, sampler);

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = texture;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::Texture:
            {
                ComPtr<ITextureResource> texture;
                SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(entry.textureDesc, textureBindFlags, renderer, texture));
                resource = texture;

                // TODO: support UAV textures...

                IResourceView::Desc viewDesc;
                viewDesc.type = IResourceView::Type::ShaderResource;
                auto textureView = renderer->createTextureView(
                    texture,
                    viewDesc);

                if (!textureView)
                {
                    return SLANG_FAIL;
                }

                entryCursor.setResource(textureView);

#if 0
                descriptorSet->setResource(rangeIndex, 0, textureView);

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = texture;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::Sampler:
            {
                auto sampler = _createSamplerState(renderer, entry.samplerDesc);

                entryCursor.setSampler(sampler);
#if 0
                descriptorSet->setSampler(rangeIndex, 0, sampler);
#endif
            }
            break;

        case ShaderInputType::Object:
            {
                auto typeName = entry.objectDesc.typeName;
                slang::TypeLayoutReflection* slangTypeLayout = nullptr;
                if(typeName.getLength() != 0)
                {
                    // If the input line specified the name of the type
                    // to allocate, then we use it directly.
                    //
                    auto slangType = slangReflection->findTypeByName(typeName.getBuffer());
                    slangTypeLayout = slangReflection->getTypeLayout(slangType);
                }
                else
                {
                    // if the user did not specify what type to allocate,
                    // then we will infer the type from the type of the
                    // value pointed to by `entryCursor`.
                    //
                    slangTypeLayout = entryCursor.getTypeLayout();
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
                }

                ComPtr<IShaderObjectLayout> shaderObjectLayout = renderer->createShaderObjectLayout(slangTypeLayout);
                ComPtr<IShaderObject> shaderObject =
                    renderer->createShaderObject(shaderObjectLayout);

                entryCursor.setObject(shaderObject);
            }
            break;

        default:
            assert(!"Unhandled type");
            return SLANG_FAIL;
        }

        if(entry.isOutput)
        {
            ShaderOutputPlan::Item item;
            item.inputLayoutEntryIndex = entryIndex;
            item.resource = resource;
            ioOutputPlan.items.add(item);
        }

    }
    return SLANG_OK;
}

void LegacyRenderTestApp::applyBinding(PipelineType pipelineType)
{
    m_bindingState->apply(m_renderer.get(), pipelineType);
}

void ShaderObjectRenderTestApp::applyBinding(PipelineType pipelineType)
{
    m_renderer->bindRootShaderObject(pipelineType, m_programVars);
}

SlangResult LegacyRenderTestApp::initialize(
    SlangSession* session,
    IRenderer* renderer,
    const Options& options,
    const ShaderCompilerUtil::Input& input)
{
    m_options = options;

    SLANG_RETURN_ON_FAIL(_initializeShaders(session, renderer, options.shaderType, input));

    m_numAddedConstantBuffers = 0;
    m_renderer = renderer;

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    m_constantBufferSize = 16 * sizeof(float);

    IBufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(m_constantBufferSize);
    constantBufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;

    m_constantBuffer =
        renderer->createBufferResource(IResource::Usage::ConstantBuffer, constantBufferDesc);
    if (!m_constantBuffer)
        return SLANG_FAIL;

    //! Hack -> if doing a graphics test, add an extra binding for our dynamic constant buffer
    //
    // TODO: Should probably be more sophisticated than this - with 'dynamic' constant buffer/s
    // binding always being specified in the test file
    ComPtr<IBufferResource> addedConstantBuffer;
    switch (m_options.shaderType)
    {
    default:
        break;

    case Options::ShaderProgramType::Graphics:
    case Options::ShaderProgramType::GraphicsCompute:
        addedConstantBuffer = m_constantBuffer;
        m_numAddedConstantBuffers++;
        break;
    }

    BindingStateImpl* bindingState = nullptr;
    SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBindingState(
        m_shaderInputLayout, m_renderer, addedConstantBuffer, &bindingState));
    m_bindingState = bindingState;

    // Do other initialization that doesn't depend on the source language.

    // Input Assembler (IA)

    const InputElementDesc inputElements[] = {
        {"A", 0, Format::RGB_Float32, offsetof(Vertex, position)},
        {"A", 1, Format::RGB_Float32, offsetof(Vertex, color)},
        {"A", 2, Format::RG_Float32, offsetof(Vertex, uv)},
    };

    m_inputLayout = renderer->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements));
    if (!m_inputLayout)
        return SLANG_FAIL;

    IBufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

    m_vertexBuffer = renderer->createBufferResource(
        IResource::Usage::VertexBuffer, vertexBufferDesc, kVertexData);
    if (!m_vertexBuffer)
        return SLANG_FAIL;

    {
        switch (m_options.shaderType)
        {
        default:
            assert(!"unexpected test shader type");
            return SLANG_FAIL;

        case Options::ShaderProgramType::Compute:
            {
                ComputePipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;

                m_pipelineState = renderer->createComputePipelineState(desc);
            }
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            {
                GraphicsPipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;
                desc.inputLayout = m_inputLayout;
                desc.renderTargetCount = m_bindingState->m_numRenderTargets;

                m_pipelineState = renderer->createGraphicsPipelineState(desc);
            }
            break;
        }
    }

    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

SlangResult ShaderObjectRenderTestApp::initialize(
    SlangSession* session,
    IRenderer* renderer,
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
    SLANG_RETURN_ON_FAIL(renderer->createProgram(m_compilationOutput.output.desc, m_shaderProgram.writeRef()));

    // If we are doing a non-pass-through compilation, then we will rely on
    // Slang's reflection API to tell us what the parameters of the program are.
    //
    auto slangReflection = (slang::ProgramLayout*) spGetReflection(m_compilationOutput.output.getRequestForReflection());

    // Once we have determined the layout of all the parameters we need to bind,
    // we will create a shader object to use for storing and binding those parameters.
    //
    m_programVars = renderer->createRootShaderObject(m_shaderProgram);

    // Now we need to assign from the input parameter data that was parsed into
    // the program vars we allocated.
    //
    SLANG_RETURN_ON_FAIL(_assignVarsFromLayout(
        renderer, m_programVars, m_compilationOutput.layout, m_outputPlan, slangReflection));

	m_renderer = renderer;

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

                m_pipelineState = renderer->createComputePipelineState(desc);
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
                SLANG_RETURN_ON_FAIL(renderer->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements), inputLayout.writeRef()));

                IBufferResource::Desc vertexBufferDesc;
                vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

                SLANG_RETURN_ON_FAIL(renderer->createBufferResource(IResource::Usage::VertexBuffer, vertexBufferDesc, kVertexData, m_vertexBuffer.writeRef()));

                GraphicsPipelineStateDesc desc;
                desc.program = m_shaderProgram;
                desc.inputLayout = inputLayout;

                m_pipelineState = renderer->createGraphicsPipelineState(desc);
            }
            break;
        }
    }

    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

void ShaderObjectRenderTestApp::finalizeImpl()
{
    m_programVars = nullptr;
    RenderTestApp::finalizeImpl();
}

Result RenderTestApp::_initializeShaders(
    SlangSession* session,
    IRenderer* renderer,
    Options::ShaderProgramType shaderType,
    const ShaderCompilerUtil::Input& input)
{
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, m_options, input,  m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;
    m_shaderProgram = renderer->createProgram(m_compilationOutput.output.desc);
    return m_shaderProgram ? SLANG_OK : SLANG_FAIL;
}


void LegacyRenderTestApp::setProjectionMatrix()
{
    auto mappedData = m_renderer->map(m_constantBuffer, MapFlavor::WriteDiscard);
    if (mappedData)
    {
        const ProjectionStyle projectionStyle =
            gfxGetProjectionStyle(m_renderer->getRendererType());
        gfxGetIdentityProjection(projectionStyle, (float*)mappedData);

        m_renderer->unmap(m_constantBuffer);
    }
}

void ShaderObjectRenderTestApp::setProjectionMatrix()
{
    const ProjectionStyle projectionStyle =
        gfxGetProjectionStyle(m_renderer->getRendererType());

    float projectionMatrix[16];
    gfxGetIdentityProjection(projectionStyle, projectionMatrix);
    ShaderCursor(m_programVars)
        .getField("Uniforms")
        .getDereferenced()
        .setData(projectionMatrix, sizeof(projectionMatrix));
}

void RenderTestApp::renderFrame()
{
    setProjectionMatrix();

    auto pipelineType = PipelineType::Graphics;

    m_renderer->setPipelineState(pipelineType, m_pipelineState);

	m_renderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_renderer->setVertexBuffer(0, m_vertexBuffer, sizeof(Vertex));

    applyBinding(pipelineType);

	m_renderer->draw(3);
}

void RenderTestApp::runCompute()
{
    auto pipelineType = PipelineType::Compute;
    m_renderer->setPipelineState(pipelineType, m_pipelineState);
    applyBinding(pipelineType);

    m_startTicks = ProcessUtil::getClockTick();

	m_renderer->dispatchCompute(m_options.computeDispatchSize[0], m_options.computeDispatchSize[1], m_options.computeDispatchSize[2]);
}

void RenderTestApp::finalize()
{
    finalizeImpl();

    m_inputLayout = nullptr;
    m_vertexBuffer = nullptr;
    m_shaderProgram = nullptr;
    m_pipelineState = nullptr;

    m_renderer = nullptr;
}

void RenderTestApp::finalizeImpl()
{
}

Result LegacyRenderTestApp::writeBindingOutput(BindRoot* bindRoot, const char* fileName)
{
    // Submit the work
    m_renderer->submitGpuWork();
    // Wait until everything is complete
    m_renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }
    FileWriter writer(f, WriterFlags(0));

    for(auto binding : m_bindingState->outputBindings)
    {
        auto i = binding.entryIndex;
        const auto& layoutBinding = m_shaderInputLayout.entries[i];

        assert(layoutBinding.isOutput);
        
        if (binding.resource && binding.resource->getType() == IResource::Type::Buffer)
        {
            IBufferResource* bufferResource = static_cast<IBufferResource*>(binding.resource.get());
            const size_t bufferSize = bufferResource->getDesc()->sizeInBytes;

            unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
            if (!ptr)
            {
                return SLANG_FAIL;
            }

            const SlangResult res = ShaderInputLayout::writeBinding(bindRoot, m_shaderInputLayout.entries[i], ptr, bufferSize, &writer);

            m_renderer->unmap(bufferResource);

            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            printf("invalid output type at %d.\n", int(i));
        }
    }
    
    return SLANG_OK;
}

Result ShaderObjectRenderTestApp::writeBindingOutput(BindRoot* bindRoot, const char* fileName)
{
    // Submit the work
    m_renderer->submitGpuWork();
    // Wait until everything is complete
    m_renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }
    FileWriter writer(f, WriterFlags(0));

    for(auto outputItem : m_outputPlan.items)
    {
        auto& inputEntry = m_shaderInputLayout.entries[outputItem.inputLayoutEntryIndex];
        assert(inputEntry.isOutput);

        auto resource = outputItem.resource;
        if (resource && resource->getType() == IResource::Type::Buffer)
        {
            IBufferResource* bufferResource = static_cast<IBufferResource*>(resource.get());
            const size_t bufferSize = bufferResource->getDesc()->sizeInBytes;

            unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
            if (!ptr)
            {
                return SLANG_FAIL;
            }

            const SlangResult res = ShaderInputLayout::writeBinding(bindRoot, inputEntry, ptr, bufferSize, &writer);

            m_renderer->unmap(bufferResource);

            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            printf("invalid output type at %d.\n", int(outputItem.inputLayoutEntryIndex));
        }
    }
    return SLANG_OK;
}


Result RenderTestApp::writeScreen(const char* filename)
{
    size_t rowPitch, bufferSize, pixelSize;
    List<uint8_t> buffer;

    SLANG_RETURN_ON_FAIL(m_renderer->captureScreenSurface(nullptr, &bufferSize, &rowPitch, &pixelSize));
    buffer.setCount(bufferSize);
    SLANG_RETURN_ON_FAIL(
        m_renderer->captureScreenSurface(buffer.getBuffer(), &bufferSize, &rowPitch, &pixelSize));

    Surface surface;
    size_t width = rowPitch / pixelSize;
    size_t height = bufferSize / rowPitch;
    surface.setUnowned(
        (int)width,
        (int)height,
        gfx::Format::RGBA_Unorm_UInt8,
        (int)rowPitch,
        buffer.getBuffer());
    return PngSerializeUtil::write(filename, surface);
}

Result RenderTestApp::update(Window* window)
{
    // Whenever we don't have Windows events to process, we render a frame.
    if (m_options.shaderType == Options::ShaderProgramType::Compute)
    {
        runCompute();
    }
    else
    {
        static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
        m_renderer->setClearColor(kClearColor);
        m_renderer->clearFrame();

        renderFrame();
    }

    // If we are in a mode where output is requested, we need to snapshot the back buffer here
    if (m_options.outputPath || m_options.performanceProfile)
    {
        // Submit the work
        m_renderer->submitGpuWork();
        // Wait until everything is complete
        m_renderer->waitForGpu();

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

                BindSet bindSet;
                GPULikeBindRoot bindRoot;
                bindRoot.init(&bindSet, slangReflection, 0);

                BindRoot* outputBindRoot = m_options.outputUsingType ? &bindRoot : nullptr;

                SLANG_RETURN_ON_FAIL(writeBindingOutput(outputBindRoot, m_options.outputPath));
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
        // We are done
        window->postQuit();
        return SLANG_OK;
    }

    m_renderer->presentFrame();
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

static SlangResult _innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace renderer_test;
    using namespace Slang;

    StdWriters::setSingleton(stdWriters);

    Options options;

	// Parse command-line options
	SLANG_RETURN_ON_FAIL(Options::parse(argcIn, argvIn, StdWriters::getError(), options));

    // Declare window pointer before renderer, such that window is released after renderer
    RefPtr<renderer_test::Window> window;

    ShaderCompilerUtil::Input input;
    
    input.profile = "";
    input.target = SLANG_TARGET_NONE;
    input.args = &options.slangArgs[0];
    input.argCount = options.slangArgCount;

	SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
	SlangPassThrough slangPassThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
	switch (options.rendererType)
	{
		case RendererType::DirectX11:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
			break;

		case RendererType::DirectX12:
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

		case RendererType::OpenGl:
			input.target = SLANG_GLSL;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;

		case RendererType::Vulkan:
			input.target = SLANG_SPIRV;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;
        case RendererType::CPU:
            input.target = SLANG_HOST_CALLABLE;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            slangPassThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
        case RendererType::CUDA:
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
    rendererName << "[" << gfxGetRendererName(options.rendererType) << "] ";
    if (options.adapter.getLength())
    {
        rendererName << "'" << options.adapter << "'";
    }

    if (options.onlyStartup)
    {
        switch (options.rendererType)
        {
            case RendererType::CUDA:
            {
#if RENDER_TEST_CUDA
                return SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_NVRTC)) && CUDAComputeUtil::canCreateDevice() ? SLANG_OK : SLANG_FAIL;
#else
                return SLANG_FAIL;
#endif
            }
            case RendererType::CPU:
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

    // If it's CPU testing we don't need a window or a renderer
    if (options.rendererType == RendererType::CPU)
    {
        // Check we have all the required features
        for (const auto& renderFeature : options.renderFeatures)
        {
            if (!CPUComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, compilationAndLayout));

        {
            // Get the shared library -> it contains the executable code, we need to keep around if we recompile
            ComPtr<ISlangSharedLibrary> sharedLibrary;
            SLANG_RETURN_ON_FAIL(spGetEntryPointHostCallable(compilationAndLayout.output.getRequestForKernels(), 0, 0, sharedLibrary.writeRef()));

            // This is a hack to work around, reflection when compiling straight C/C++ code. In that case the code is just passed
            // straight through to the C++ compiler so no reflection. In these tests though we should have conditional code
            // (performance-profile.slang for example), such that there is both a slang and C++ code, and it is the job
            // of the test implementer to *ensure* that the straight C++ code has the same layout as the slang C++ backend.
            //
            // If we are running c/c++ we still need binding information, so compile again as slang source
            if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
            {
                ShaderCompilerUtil::Input slangInput = input;
                slangInput.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
                slangInput.passThrough = SLANG_PASS_THROUGH_NONE;
                // We just want CPP, so we get suitable reflection
                slangInput.target = SLANG_CPP_SOURCE;

                SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, slangInput, compilationAndLayout));
            }

            // calculate binding
            CPUComputeUtil::Context context;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::createBindlessResources(compilationAndLayout, context));
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::fillRuntimeHandleInBuffers(compilationAndLayout, context, sharedLibrary.get()));
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, context));

            // Get the execution info from the lib
            CPUComputeUtil::ExecuteInfo info;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcExecuteInfo(CPUComputeUtil::ExecuteStyle::GroupRange, sharedLibrary, options.computeDispatchSize, compilationAndLayout, context, info));

            const uint64_t startTicks = ProcessUtil::getClockTick();

            SLANG_RETURN_ON_FAIL(CPUComputeUtil::execute(info));

            if (options.performanceProfile)
            {
                const uint64_t endTicks = ProcessUtil::getClockTick();
                _outputProfileTime(startTicks, endTicks);
            }

            if (options.outputPath)
            {
                BindRoot* outputBindRoot = options.outputUsingType ? &context.m_bindRoot : nullptr;


                // Dump everything out that was written
                SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, options.outputPath));

                // Check all execution styles produce the same result
                SLANG_RETURN_ON_FAIL(CPUComputeUtil::checkStyleConsistency(sharedLibrary, options.computeDispatchSize, compilationAndLayout));
            }
        }

        return SLANG_OK;
    }

    if (options.rendererType == RendererType::CUDA && !options.useShaderObjects)
    {        
#if RENDER_TEST_CUDA
        // Check we have all the required features
        for (const auto& renderFeature : options.renderFeatures)
        {
            if (!CUDAComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, compilationAndLayout));

        const uint64_t startTicks = ProcessUtil::getClockTick();

        CUDAComputeUtil::Context context;
        SLANG_RETURN_ON_FAIL(CUDAComputeUtil::execute(compilationAndLayout, options.computeDispatchSize, context));

        if (options.performanceProfile)
        {
            const uint64_t endTicks = ProcessUtil::getClockTick();
            _outputProfileTime(startTicks, endTicks);
        }

        if (options.outputPath)
        {
            BindRoot* outputBindRoot = options.outputUsingType ? &context.m_bindRoot : nullptr;

            // Dump everything out that was written
            SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, options.outputPath));
        }

        return SLANG_OK;
#else
        return SLANG_FAIL;
#endif
    }

    Slang::ComPtr<IRenderer> renderer;
    {
        SGRendererCreateFunc createFunc = gfxGetCreateFunc(options.rendererType);
        if (createFunc)
        {
            createFunc(renderer.writeRef());
        }

        if (!renderer)
        {
            if (!options.onlyStartup)
            {
                fprintf(stderr, "Unable to create renderer %s\n", rendererName.getBuffer());
            }
            return SLANG_FAIL;
        }

        IRenderer::Desc desc;
        desc.width = gWindowWidth;
        desc.height = gWindowHeight;
        desc.adapter = options.adapter.getBuffer();
        List<const char*> requiredFeatureList;
        for (auto & name : options.renderFeatures)
            requiredFeatureList.add(name.getBuffer());
        desc.requiredFeatures = requiredFeatureList.getBuffer();
        desc.requiredFeatureCount = (int)requiredFeatureList.getCount();
        desc.nvapiExtnSlot = int(nvapiExtnSlot);

        window = renderer_test::Window::create();
        SLANG_RETURN_ON_FAIL(window->initialize(gWindowWidth, gWindowHeight));

        SlangResult res = renderer->initialize(desc, window->getHandle());
        if (SLANG_FAILED(res))
        {
            // Returns E_NOT_AVAILABLE only when specified features are not available.
            // Will cause to be ignored.
            if (!options.onlyStartup && res != SLANG_E_NOT_AVAILABLE)
            {
                fprintf(stderr, "Unable to initialize renderer %s\n", rendererName.getBuffer());
            }
            return res;
        }

        for (const auto& feature : requiredFeatureList)
        {
            // If doesn't have required feature... we have to give up
            if (!renderer->hasFeature(feature))
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
        RefPtr<RenderTestApp> app;
        if (options.useShaderObjects)
            app = new ShaderObjectRenderTestApp();
        else
            app = new LegacyRenderTestApp();
		SLANG_RETURN_ON_FAIL(app->initialize(session, renderer, options, input));
        window->show();
        SLANG_RETURN_ON_FAIL(window->runLoop(app));
        app->finalize();
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

