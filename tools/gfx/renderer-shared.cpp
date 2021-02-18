#include "renderer-shared.h"
#include "render-graphics-common.h"
#include "core/slang-io.h"
#include "core/slang-token-reader.h"

using namespace Slang;

namespace gfx
{

const Slang::Guid GfxGUID::IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
const Slang::Guid GfxGUID::IID_IDescriptorSetLayout = SLANG_UUID_IDescriptorSetLayout;
const Slang::Guid GfxGUID::IID_IDescriptorSet = SLANG_UUID_IDescriptorSet;
const Slang::Guid GfxGUID::IID_IShaderProgram = SLANG_UUID_IShaderProgram;
const Slang::Guid GfxGUID::IID_IPipelineLayout = SLANG_UUID_IPipelineLayout;
const Slang::Guid GfxGUID::IID_IInputLayout = SLANG_UUID_IInputLayout;
const Slang::Guid GfxGUID::IID_IPipelineState = SLANG_UUID_IPipelineState;
const Slang::Guid GfxGUID::IID_IResourceView = SLANG_UUID_IResourceView;
const Slang::Guid GfxGUID::IID_ISamplerState = SLANG_UUID_ISamplerState;
const Slang::Guid GfxGUID::IID_IResource = SLANG_UUID_IResource;
const Slang::Guid GfxGUID::IID_IBufferResource = SLANG_UUID_IBufferResource;
const Slang::Guid GfxGUID::IID_ITextureResource = SLANG_UUID_ITextureResource;
const Slang::Guid GfxGUID::IID_IRenderer = SLANG_UUID_IRenderer;
const Slang::Guid GfxGUID::IID_IShaderObject = SLANG_UUID_IShaderObject;

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

Result createProgramFromSlang(IRenderer* renderer, IShaderProgram::Desc const& originalDesc, IShaderProgram** outProgram)
{
    SlangInt targetIndex = 0;
    auto slangProgram = originalDesc.slangProgram;

    auto programLayout = slangProgram->getLayout(targetIndex);
    if(!programLayout)
        return SLANG_FAIL;

    Int entryPointCount = (Int) programLayout->getEntryPointCount();
    if(entryPointCount == 0)
        return SLANG_FAIL;

    List<IShaderProgram::KernelDesc> kernelDescs;
    List<ComPtr<slang::IBlob>> kernelBlobs;
    for( Int i = 0; i < entryPointCount; ++i )
    {
        ComPtr<slang::IBlob> entryPointCodeBlob;
        SLANG_RETURN_ON_FAIL(slangProgram->getEntryPointCode(i, targetIndex, entryPointCodeBlob.writeRef()));

        auto entryPointLayout = programLayout->getEntryPointByIndex(i);

        kernelBlobs.add(entryPointCodeBlob);

        IShaderProgram::KernelDesc kernelDesc;
        kernelDesc.codeBegin = entryPointCodeBlob->getBufferPointer();
        kernelDesc.codeEnd = (const char*) kernelDesc.codeBegin + entryPointCodeBlob->getBufferSize();
        kernelDesc.entryPointName = entryPointLayout->getName();
        kernelDesc.stage = mapStage(entryPointLayout->getStage());

        kernelDescs.add(kernelDesc);
    }
    SLANG_ASSERT(kernelDescs.getCount() == entryPointCount);

    IShaderProgram::Desc programDesc;
    programDesc.pipelineType = originalDesc.pipelineType;
    programDesc.slangProgram = slangProgram;
    programDesc.kernelCount = kernelDescs.getCount();
    programDesc.kernels = kernelDescs.getBuffer();

    return renderer->createProgram(programDesc, outProgram);
}

IShaderObject* gfx::ShaderObjectBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderObject)
        return static_cast<IShaderObject*>(this);
    return nullptr;
}

IShaderProgram* gfx::ShaderProgramBase::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderProgram)
        return static_cast<IShaderProgram*>(this);
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
    isSpecializable = (program->slangProgram && program->slangProgram->getSpecializationParamCount() != 0);
}

IRenderer* gfx::RendererBase::getInterface(const Guid& guid)
{
    return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderer)
        ? static_cast<IRenderer*>(this)
        : nullptr;
}

SLANG_NO_THROW Result SLANG_MCALL RendererBase::initialize(const Desc& desc, void* inWindowHandle)
{
    SLANG_UNUSED(inWindowHandle);

    shaderCache.init(desc.shaderCacheFileSystem);
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

void ShaderCache::init(ISlangFileSystem* cacheFileSystem)
{
    fileSystem = cacheFileSystem;

    ComPtr<ISlangBlob> indexFileBlob;
    if (fileSystem && fileSystem->loadFile("index", indexFileBlob.writeRef()) == SLANG_OK)
    {
        UnownedStringSlice indexText = UnownedStringSlice(static_cast<const char*>(indexFileBlob->getBufferPointer()));
        TokenReader reader = TokenReader(indexText);
        auto componentCountInFileSystem = reader.ReadUInt();
        for (uint32_t i = 0; i < componentCountInFileSystem; i++)
        {
            OwningComponentKey key;
            auto componentId = reader.ReadUInt();
            key.typeName = reader.ReadWord();
            key.specializationArgs.setCount(reader.ReadUInt());
            for (auto& arg : key.specializationArgs)
                arg = reader.ReadUInt();
            componentIds[key] = componentId;
        }
    }
}

void ShaderCache::writeToFileSystem(ISlangMutableFileSystem* outputFileSystem)
{
    StringBuilder indexBuilder;
    indexBuilder << componentIds.Count() << Slang::EndLine;
    for (auto id : componentIds)
    {
        indexBuilder << id.Value << " ";
        indexBuilder << id.Key.typeName << " " << id.Key.specializationArgs.getCount();
        for (auto arg : id.Key.specializationArgs)
            indexBuilder << " " << arg;
        indexBuilder << Slang::EndLine;
    }
    outputFileSystem->saveFile("index", indexBuilder.getBuffer(), indexBuilder.getLength());
    for (auto& binary : shaderBinaries)
    {
        ComPtr<ISlangBlob> blob;
        binary.Value->writeToBlob(blob.writeRef());
        outputFileSystem->saveFile(String(binary.Key).getBuffer(), blob->getBufferPointer(), blob->getBufferSize());
    }
}

Slang::RefPtr<ShaderBinary> ShaderCache::tryLoadShaderBinary(ShaderComponentID componentId)
{
    Slang::ComPtr<ISlangBlob> entryBlob;
    Slang::RefPtr<ShaderBinary> binary;
    if (shaderBinaries.TryGetValue(componentId, binary))
        return binary;

    if (fileSystem && fileSystem->loadFile(String(componentId).getBuffer(), entryBlob.writeRef()) == SLANG_OK)
    {
        binary = new ShaderBinary();
        binary->loadFromBlob(entryBlob.get());
        return binary;
    }
    return nullptr;
}

void ShaderCache::addShaderBinary(ShaderComponentID componentId, ShaderBinary* binary)
{
    shaderBinaries[componentId] = binary;
}

void ShaderCache::addSpecializedPipeline(PipelineKey key, Slang::ComPtr<IPipelineState> specializedPipeline)
{
    specializedPipelines[key] = specializedPipeline;
}

struct ShaderBinaryEntryHeader
{
    StageType stage;
    uint32_t nameLength;
    uint32_t codeLength;
};

Result ShaderBinary::loadFromBlob(ISlangBlob* blob)
{
    MemoryStreamBase memStream(Slang::FileAccess::Read, blob->getBufferPointer(), blob->getBufferSize());
    uint32_t nameLength = 0;
    ShaderBinaryEntryHeader header;
    if (memStream.read(&header, sizeof(header)) != sizeof(header))
        return SLANG_FAIL;
    const uint8_t* name = memStream.getContents().getBuffer() + memStream.getPosition();
    const uint8_t* code = name + header.nameLength;
    entryPointName = reinterpret_cast<const char*>(name);
    stage = header.stage;
    source.addRange(code, header.codeLength);
    return SLANG_OK;
}

Result ShaderBinary::writeToBlob(ISlangBlob** outBlob)
{
    OwnedMemoryStream outStream(FileAccess::Write);
    ShaderBinaryEntryHeader header;
    header.stage = stage;
    header.nameLength = static_cast<uint32_t>(entryPointName.getLength() + 1);
    header.codeLength = static_cast<uint32_t>(source.getCount());
    outStream.write(&header, sizeof(header));
    outStream.write(entryPointName.getBuffer(), header.nameLength - 1);
    uint8_t zeroTerminator = 0;
    outStream.write(&zeroTerminator, 1);
    outStream.write(source.getBuffer(), header.codeLength);
    RefPtr<RawBlob> blob = new RawBlob(outStream.getContents().getBuffer(), outStream.getContents().getCount());
    *outBlob = blob.detach();
    return SLANG_OK;
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

Result RendererBase::maybeSpecializePipeline(ShaderObjectBase* rootObject)
{
    auto currentPipeline = getCurrentPipeline();
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

        ComPtr<gfx::IPipelineState> specializedPipelineState = shaderCache.getSpecializedPipelineState(pipelineKey);
        // Try to find specialized pipeline from shader cache.
        if (!specializedPipelineState)
        {
            auto unspecializedProgram = static_cast<ShaderProgramBase*>(pipelineType == PipelineType::Compute
                ? currentPipeline->desc.compute.program
                : currentPipeline->desc.graphics.program);
            List<RefPtr<ShaderBinary>> entryPointBinaries;
            auto unspecializedProgramLayout = unspecializedProgram->slangProgram->getLayout();
            for (SlangUInt i = 0; i < unspecializedProgramLayout->getEntryPointCount(); i++)
            {
                auto unspecializedEntryPoint = unspecializedProgramLayout->getEntryPointByIndex(i);
                UnownedStringSlice entryPointName = UnownedStringSlice(unspecializedEntryPoint->getName());
                ComponentKey specializedKernelKey;
                specializedKernelKey.typeName = entryPointName;
                specializedKernelKey.specializationArgs.addRange(specializationArgs.componentIDs);
                specializedKernelKey.updateHash();
                // If the pipeline is not created, check if the kernel binaries has been compiled.
                auto specializedKernelComponentID = shaderCache.getComponentId(specializedKernelKey);
                RefPtr<ShaderBinary> binary = shaderCache.tryLoadShaderBinary(specializedKernelComponentID);
                if (!binary)
                {
                    // If the specialized shader binary does not exist in cache, use slang to generate it.
                    entryPointBinaries.clear();
                    ComPtr<slang::IComponentType> specializedComponentType;
                    ComPtr<slang::IBlob> diagnosticBlob;
                    auto result = unspecializedProgram->slangProgram->specialize(specializationArgs.components.getArrayView().getBuffer(),
                        specializationArgs.getCount(), specializedComponentType.writeRef(), diagnosticBlob.writeRef());

                    // TODO: print diagnostic message via debug output interface.

                    if (result != SLANG_OK)
                        return result;

                    // Cache specialized binaries.
                    auto programLayout = specializedComponentType->getLayout();
                    for (SlangUInt j = 0; j < programLayout->getEntryPointCount(); j++)
                    {
                        auto entryPointLayout = programLayout->getEntryPointByIndex(j);
                        ComPtr<slang::IBlob> entryPointCode;
                        SLANG_RETURN_ON_FAIL(specializedComponentType->getEntryPointCode(j, 0, entryPointCode.writeRef(), diagnosticBlob.writeRef()));
                        binary = new ShaderBinary();
                        binary->stage = gfx::translateStage(entryPointLayout->getStage());
                        binary->entryPointName = entryPointLayout->getName();
                        binary->source.addRange((uint8_t*)entryPointCode->getBufferPointer(), entryPointCode->getBufferSize());
                        entryPointBinaries.add(binary);
                        shaderCache.addShaderBinary(specializedKernelComponentID, binary);
                    }

                    // We have already obtained all kernel binaries from this program, so break out of the outer loop since we no longer
                    // need to examine the rest of the kernels.
                    break;
                }
                entryPointBinaries.add(binary);
            }

            // Now create specialized shader program using compiled binaries.
            ComPtr<IShaderProgram> specializedProgram;
            IShaderProgram::Desc specializedProgramDesc = {};
            specializedProgramDesc.kernelCount = unspecializedProgramLayout->getEntryPointCount();
            ShortList<IShaderProgram::KernelDesc> kernelDescs;
            kernelDescs.setCount(entryPointBinaries.getCount());
            for (Slang::Index i = 0; i < entryPointBinaries.getCount(); i++)
            {
                auto entryPoint = unspecializedProgramLayout->getEntryPointByIndex(i);;
                auto& kernelDesc = kernelDescs[i];
                kernelDesc.stage = entryPointBinaries[i]->stage;
                kernelDesc.entryPointName = entryPointBinaries[i]->entryPointName.getBuffer();
                kernelDesc.codeBegin = entryPointBinaries[i]->source.begin();
                kernelDesc.codeEnd = entryPointBinaries[i]->source.end();
            }
            specializedProgramDesc.kernels = kernelDescs.getArrayView().getBuffer();
            specializedProgramDesc.pipelineType = pipelineType;
            SLANG_RETURN_ON_FAIL(createProgram(specializedProgramDesc, specializedProgram.writeRef()));

            // Create specialized pipeline state.
            switch (pipelineType)
            {
            case PipelineType::Compute:
            {
                auto pipelineDesc = currentPipeline->desc.compute;
                pipelineDesc.program = specializedProgram;
                SLANG_RETURN_ON_FAIL(createComputePipelineState(pipelineDesc, specializedPipelineState.writeRef()));
                break;
            }
            case PipelineType::Graphics:
            {
                auto pipelineDesc = currentPipeline->desc.graphics;
                pipelineDesc.program = specializedProgram;
                SLANG_RETURN_ON_FAIL(createGraphicsPipelineState(pipelineDesc, specializedPipelineState.writeRef()));
                break;
            }
            default:
                break;
            }
            auto specializedPipelineStateBase = static_cast<PipelineStateBase*>(specializedPipelineState.get());
            specializedPipelineStateBase->unspecializedPipelineState = currentPipeline;
            shaderCache.addSpecializedPipeline(pipelineKey, specializedPipelineState);
        }
        setPipelineState(specializedPipelineState);
    }
    return SLANG_OK;
}


} // namespace gfx
