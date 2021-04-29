#include "renderer-shared.h"
#include "core/slang-io.h"
#include "core/slang-token-reader.h"

using namespace Slang;

namespace gfx
{

const Slang::Guid GfxGUID::IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
const Slang::Guid GfxGUID::IID_IShaderProgram = SLANG_UUID_IShaderProgram;
const Slang::Guid GfxGUID::IID_IInputLayout = SLANG_UUID_IInputLayout;
const Slang::Guid GfxGUID::IID_IPipelineState = SLANG_UUID_IPipelineState;
const Slang::Guid GfxGUID::IID_ITransientResourceHeap = SLANG_UUID_ITransientResourceHeap;
const Slang::Guid GfxGUID::IID_IResourceView = SLANG_UUID_IResourceView;
const Slang::Guid GfxGUID::IID_IFramebuffer = SLANG_UUID_IFrameBuffer;
const Slang::Guid GfxGUID::IID_IFramebufferLayout = SLANG_UUID_IFramebufferLayout;

const Slang::Guid GfxGUID::IID_ISwapchain = SLANG_UUID_ISwapchain;
const Slang::Guid GfxGUID::IID_ISamplerState = SLANG_UUID_ISamplerState;
const Slang::Guid GfxGUID::IID_IResource = SLANG_UUID_IResource;
const Slang::Guid GfxGUID::IID_IBufferResource = SLANG_UUID_IBufferResource;
const Slang::Guid GfxGUID::IID_ITextureResource = SLANG_UUID_ITextureResource;
const Slang::Guid GfxGUID::IID_IDevice = SLANG_UUID_IDevice;
const Slang::Guid GfxGUID::IID_IShaderObject = SLANG_UUID_IShaderObject;

const Slang::Guid GfxGUID::IID_IRenderPassLayout = SLANG_UUID_IRenderPassLayout;
const Slang::Guid GfxGUID::IID_ICommandEncoder = SLANG_UUID_ICommandEncoder;
const Slang::Guid GfxGUID::IID_IRenderCommandEncoder = SLANG_UUID_IRenderCommandEncoder;
const Slang::Guid GfxGUID::IID_IComputeCommandEncoder = SLANG_UUID_IComputeCommandEncoder;
const Slang::Guid GfxGUID::IID_IResourceCommandEncoder = SLANG_UUID_IResourceCommandEncoder;
const Slang::Guid GfxGUID::IID_ICommandBuffer = SLANG_UUID_ICommandBuffer;
const Slang::Guid GfxGUID::IID_ICommandQueue = SLANG_UUID_ICommandQueue;

gfx::StageType translateStage(SlangStage slangStage)
{
    switch (slangStage)
    {
    default:
        SLANG_ASSERT(!"unhandled case");
        return gfx::StageType::Unknown;

#define CASE(FROM, TO)       \
case SLANG_STAGE_##FROM: \
return gfx::StageType::TO

        CASE(VERTEX, Vertex);
        CASE(HULL, Hull);
        CASE(DOMAIN, Domain);
        CASE(GEOMETRY, Geometry);
        CASE(FRAGMENT, Fragment);

        CASE(COMPUTE, Compute);

        CASE(RAY_GENERATION, RayGeneration);
        CASE(INTERSECTION, Intersection);
        CASE(ANY_HIT, AnyHit);
        CASE(CLOSEST_HIT, ClosestHit);
        CASE(MISS, Miss);
        CASE(CALLABLE, Callable);

#undef CASE
    }
}

IResource* BufferResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_IBufferResource)
        return static_cast<IBufferResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL BufferResource::getType() { return m_type; }
SLANG_NO_THROW IBufferResource::Desc* SLANG_MCALL BufferResource::getDesc() { return &m_desc; }


IResource* TextureResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_ITextureResource)
        return static_cast<ITextureResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL TextureResource::getType() { return m_type; }
SLANG_NO_THROW ITextureResource::Desc* SLANG_MCALL TextureResource::getDesc() { return &m_desc; }

gfx::StageType mapStage(SlangStage stage)
{
    switch( stage )
    {
    default:
        return gfx::StageType::Unknown;

    case SLANG_STAGE_AMPLIFICATION:     return gfx::StageType::Amplification;
    case SLANG_STAGE_ANY_HIT:           return gfx::StageType::AnyHit;
    case SLANG_STAGE_CALLABLE:          return gfx::StageType::Callable;
    case SLANG_STAGE_CLOSEST_HIT:       return gfx::StageType::ClosestHit;
    case SLANG_STAGE_COMPUTE:           return gfx::StageType::Compute;
    case SLANG_STAGE_DOMAIN:            return gfx::StageType::Domain;
    case SLANG_STAGE_FRAGMENT:          return gfx::StageType::Fragment;
    case SLANG_STAGE_GEOMETRY:          return gfx::StageType::Geometry;
    case SLANG_STAGE_HULL:              return gfx::StageType::Hull;
    case SLANG_STAGE_INTERSECTION:      return gfx::StageType::Intersection;
    case SLANG_STAGE_MESH:              return gfx::StageType::Mesh;
    case SLANG_STAGE_MISS:              return gfx::StageType::Miss;
    case SLANG_STAGE_RAY_GENERATION:    return gfx::StageType::RayGeneration;
    case SLANG_STAGE_VERTEX:            return gfx::StageType::Vertex;
    }
}

IShaderObject* gfx::ShaderObjectBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderObject)
        return static_cast<IShaderObject*>(this);
    return nullptr;
}

bool gfx::ShaderObjectBase::_doesValueFitInExistentialPayload(
    slang::TypeLayoutReflection*    concreteTypeLayout,
    slang::TypeLayoutReflection*    existentialTypeLayout)
{
    // Our task here is to figure out if a value of `concreteTypeLayout`
    // can fit into an existential value using `existentialTypelayout`.

    // We can start by asking how many bytes the concrete type of the object consumes.
    //
    auto concreteValueSize = concreteTypeLayout->getSize();

    // We can also compute how many bytes the existential-type value provides,
    // but we need to remember that the *payload* part of that value comes after
    // the header with RTTI and witness-table IDs, so the payload is 16 bytes
    // smaller than the entire value.
    //
    auto existentialValueSize = existentialTypeLayout->getSize();
    auto existentialPayloadSize = existentialValueSize - 16;

    // If the concrete type consumes more ordinary bytes than we have in the payload,
    // it cannot possibly fit.
    //
    if(concreteValueSize > existentialPayloadSize)
        return false;

    // It is possible that the ordinary bytes of `concreteTypeLayout` can fit
    // in the payload, but that type might also use storage other than ordinary
    // bytes. In that case, the value would *not* fit, because all the non-ordinary
    // data can't fit in the payload at all.
    //
    auto categoryCount = concreteTypeLayout->getCategoryCount();
    for(unsigned int i = 0; i < categoryCount; ++i)
    {
        auto category = concreteTypeLayout->getCategoryByIndex(i);
        switch(category)
        {
        // We want to ignore any ordinary/uniform data usage, since that
        // was already checked above.
        //
        case slang::ParameterCategory::Uniform:
            break;

        // Any other kind of data consumed means the value cannot possibly fit.
        default:
            return false;

        // TODO: Are there any cases of resource usage that need to be ignored here?
        // E.g., if the sub-object contains its own existential-type fields (which
        // get reflected as consuming "existential value" storage) should that be
        // ignored?
        }
    }

    // If we didn't reject the concrete type above for either its ordinary
    // data or some use of non-ordinary data, then it seems like it must fit.
    //
    return true;
}

IShaderProgram* gfx::ShaderProgramBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderProgram)
        return static_cast<IShaderProgram*>(this);
    return nullptr;
}

IInputLayout* gfx::InputLayoutBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IInputLayout)
        return static_cast<IInputLayout*>(this);
    return nullptr;
}

IFramebufferLayout* gfx::FramebufferLayoutBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebufferLayout)
        return static_cast<IFramebufferLayout*>(this);
    return nullptr;
}

IPipelineState* gfx::PipelineStateBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IPipelineState)
        return static_cast<IPipelineState*>(this);
    return nullptr;
}

void PipelineStateBase::initializeBase(const PipelineStateDesc& inDesc)
{
    desc = inDesc;

    auto program = desc.getProgram();
    m_program = program;
    isSpecializable = (program->slangProgram && program->slangProgram->getSpecializationParamCount() != 0);

    // Hold a strong reference to inputLayout and framebufferLayout objects to prevent it from
    // destruction.
    if (inDesc.type == PipelineType::Graphics)
    {
        inputLayout = static_cast<InputLayoutBase*>(inDesc.graphics.inputLayout);
        framebufferLayout = static_cast<FramebufferLayoutBase*>(inDesc.graphics.framebufferLayout);
    }
}

IDevice* gfx::RendererBase::getInterface(const Guid& guid)
{
    return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IDevice)
               ? static_cast<IDevice*>(this)
               : nullptr;
}

SLANG_NO_THROW Result SLANG_MCALL RendererBase::initialize(const Desc& desc)
{
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL RendererBase::getFeatures(
    const char** outFeatures, UInt bufferSize, UInt* outFeatureCount)
{
    if (bufferSize >= (UInt)m_features.getCount())
    {
        for (Index i = 0; i < m_features.getCount(); i++)
        {
            outFeatures[i] = m_features[i].getUnownedSlice().begin();
        }
    }
    if (outFeatureCount)
        *outFeatureCount = (UInt)m_features.getCount();
    return SLANG_OK;
}

SLANG_NO_THROW bool SLANG_MCALL RendererBase::hasFeature(const char* featureName)
{
    return m_features.findFirstIndex([&](Slang::String x) { return x == featureName; }) != -1;
}

SLANG_NO_THROW Result SLANG_MCALL RendererBase::getSlangSession(slang::ISession** outSlangSession)
{
    *outSlangSession = slangContext.session.get();
    slangContext.session->addRef();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL RendererBase::createShaderObject(slang::TypeReflection* type, IShaderObject** outObject)
{
    RefPtr<ShaderObjectLayoutBase> shaderObjectLayout;
    SLANG_RETURN_FALSE_ON_FAIL(getShaderObjectLayout(type, shaderObjectLayout.writeRef()));
    return createShaderObject(shaderObjectLayout, outObject);
}

Result RendererBase::getShaderObjectLayout(
    slang::TypeReflection*      type,
    ShaderObjectLayoutBase**    outLayout)
{
    RefPtr<ShaderObjectLayoutBase> shaderObjectLayout;
    if( !m_shaderObjectLayoutCache.TryGetValue(type, shaderObjectLayout) )
    {
        auto typeLayout = slangContext.session->getTypeLayout(type);
        SLANG_RETURN_ON_FAIL(createShaderObjectLayout(typeLayout, shaderObjectLayout.writeRef()));
        m_shaderObjectLayoutCache.Add(type, shaderObjectLayout);
    }
    *outLayout = shaderObjectLayout.detach();
    return SLANG_OK;
}



ShaderComponentID ShaderCache::getComponentId(slang::TypeReflection* type)
{
    ComponentKey key;
    key.typeName = UnownedStringSlice(type->getName());
    switch (type->getKind())
    {
    case slang::TypeReflection::Kind::Specialized:
        {
            auto baseType = type->getElementType();

            StringBuilder builder;
            builder.append(UnownedTerminatedStringSlice(baseType->getName()));

            auto rawType = (SlangReflectionType*) type;

            builder.appendChar('<');
            SlangInt argCount = spReflectionType_getSpecializedTypeArgCount(rawType);
            for(SlangInt a = 0; a < argCount; ++a)
            {
                if(a != 0) builder.appendChar(',');
                if(auto rawArgType = spReflectionType_getSpecializedTypeArgType(rawType, a))
                {
                    auto argType = (slang::TypeReflection*) rawArgType;
                    builder.append(argType->getName());
                }
            }
            builder.appendChar('>');
            key.typeName = builder.getUnownedSlice();
            key.updateHash();
            return getComponentId(key);
        }
        // TODO: collect specialization arguments and append them to `key`.
        SLANG_UNIMPLEMENTED_X("specialized type");
    default:
        break;
    }
    key.updateHash();
    return getComponentId(key);
}

ShaderComponentID ShaderCache::getComponentId(UnownedStringSlice name)
{
    ComponentKey key;
    key.typeName = name;
    key.updateHash();
    return getComponentId(key);
}

ShaderComponentID ShaderCache::getComponentId(ComponentKey key)
{
    ShaderComponentID componentId = 0;
    if (componentIds.TryGetValue(key, componentId))
        return componentId;
    OwningComponentKey owningTypeKey;
    owningTypeKey.hash = key.hash;
    owningTypeKey.typeName = key.typeName;
    owningTypeKey.specializationArgs.addRange(key.specializationArgs);
    ShaderComponentID resultId = static_cast<ShaderComponentID>(componentIds.Count());
    componentIds[owningTypeKey] = resultId;
    return resultId;
}

void ShaderCache::addSpecializedPipeline(PipelineKey key, Slang::RefPtr<PipelineStateBase> specializedPipeline)
{
    specializedPipelines[key] = specializedPipeline;
}

void ShaderObjectLayoutBase::initBase(RendererBase* renderer, slang::TypeLayoutReflection* elementTypeLayout)
{
    m_renderer = renderer;
    m_elementTypeLayout = elementTypeLayout;
    m_componentID = m_renderer->shaderCache.getComponentId(m_elementTypeLayout->getType());
}

// Get the final type this shader object represents. If the shader object's type has existential fields,
// this function will return a specialized type using the bound sub-objects' type as specialization argument.
Result ShaderObjectBase::getSpecializedShaderObjectType(ExtendedShaderObjectType* outType)
{
    return _getSpecializedShaderObjectType(outType);
}

Result ShaderObjectBase::_getSpecializedShaderObjectType(ExtendedShaderObjectType* outType)
{
    if (shaderObjectType.slangType)
        *outType = shaderObjectType;
    ExtendedShaderObjectTypeList specializationArgs;
    SLANG_RETURN_ON_FAIL(collectSpecializationArgs(specializationArgs));
    if (specializationArgs.getCount() == 0)
    {
        shaderObjectType.componentID = getLayout()->getComponentID();
        shaderObjectType.slangType = getLayout()->getElementTypeLayout()->getType();
    }
    else
    {
        shaderObjectType.slangType = getRenderer()->slangContext.session->specializeType(
            getElementTypeLayout()->getType(),
            specializationArgs.components.getArrayView().getBuffer(), specializationArgs.getCount());
        shaderObjectType.componentID = getRenderer()->shaderCache.getComponentId(shaderObjectType.slangType);
    }
    *outType = shaderObjectType;
    return SLANG_OK;
}

Result RendererBase::maybeSpecializePipeline(
    PipelineStateBase* currentPipeline,
    ShaderObjectBase* rootObject,
    RefPtr<PipelineStateBase>& outNewPipeline)
{
    outNewPipeline = static_cast<PipelineStateBase*>(currentPipeline);
    
    auto pipelineType = currentPipeline->desc.type;
    if (currentPipeline->unspecializedPipelineState)
        currentPipeline = currentPipeline->unspecializedPipelineState;
    // If the currently bound pipeline is specializable, we need to specialize it based on bound shader objects.
    if (currentPipeline->isSpecializable)
    {
        specializationArgs.clear();
        SLANG_RETURN_ON_FAIL(rootObject->collectSpecializationArgs(specializationArgs));

        // Construct a shader cache key that represents the specialized shader kernels.
        PipelineKey pipelineKey;
        pipelineKey.pipeline = currentPipeline;
        pipelineKey.specializationArgs.addRange(specializationArgs.componentIDs);
        pipelineKey.updateHash();

        RefPtr<PipelineStateBase> specializedPipelineState = shaderCache.getSpecializedPipelineState(pipelineKey);
        // Try to find specialized pipeline from shader cache.
        if (!specializedPipelineState)
        {
            auto unspecializedProgram = static_cast<ShaderProgramBase*>(pipelineType == PipelineType::Compute
                ? currentPipeline->desc.compute.program
                : currentPipeline->desc.graphics.program);
            auto unspecializedProgramLayout = unspecializedProgram->slangProgram->getLayout();

            ComPtr<slang::IComponentType> specializedComponentType;
            ComPtr<slang::IBlob> diagnosticBlob;
            auto compileRs = unspecializedProgram->slangProgram->specialize(
                specializationArgs.components.getArrayView().getBuffer(),
                specializationArgs.getCount(),
                specializedComponentType.writeRef(),
                diagnosticBlob.writeRef());
            if (compileRs != SLANG_OK)
            {
                printf("%s\n", (char*)diagnosticBlob->getBufferPointer());
                return SLANG_FAIL;
            }

            // Now create specialized shader program using compiled binaries.
            ComPtr<IShaderProgram> specializedProgram;
            IShaderProgram::Desc specializedProgramDesc = {};
            specializedProgramDesc.slangProgram = specializedComponentType;
            specializedProgramDesc.pipelineType = pipelineType;
            SLANG_RETURN_ON_FAIL(createProgram(specializedProgramDesc, specializedProgram.writeRef()));

            // Create specialized pipeline state.
            ComPtr<IPipelineState> specializedPipelineComPtr;
            switch (pipelineType)
            {
            case PipelineType::Compute:
            {
                auto pipelineDesc = currentPipeline->desc.compute;
                pipelineDesc.program = specializedProgram;
                SLANG_RETURN_ON_FAIL(
                    createComputePipelineState(pipelineDesc, specializedPipelineComPtr.writeRef()));
                break;
            }
            case PipelineType::Graphics:
            {
                auto pipelineDesc = currentPipeline->desc.graphics;
                pipelineDesc.program = specializedProgram;
                SLANG_RETURN_ON_FAIL(createGraphicsPipelineState(
                    pipelineDesc, specializedPipelineComPtr.writeRef()));
                break;
            }
            default:
                break;
            }
            specializedPipelineState =
                static_cast<PipelineStateBase*>(specializedPipelineComPtr.get());
            specializedPipelineState->unspecializedPipelineState = currentPipeline;
            shaderCache.addSpecializedPipeline(pipelineKey, specializedPipelineState);
        }
        auto specializedPipelineStateBase = static_cast<PipelineStateBase*>(specializedPipelineState.Ptr());
        outNewPipeline = specializedPipelineStateBase;
    }
    return SLANG_OK;
}

IDebugCallback*& _getDebugCallback()
{
    static IDebugCallback* callback = nullptr;
    return callback;
}

class NullDebugCallback : public IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
        handleMessage(DebugMessageType type, DebugMessageSource source, const char* message) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(source);
        SLANG_UNUSED(message);
    }
};
IDebugCallback* _getNullDebugCallback()
{
    static NullDebugCallback result = {};
    return &result;
}

} // namespace gfx

