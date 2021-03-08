#pragma once

#include "slang-gfx.h"
#include "slang-context.h"
#include "core/slang-basic.h"

namespace gfx
{

struct GfxGUID
{
    static const Slang::Guid IID_ISlangUnknown;
    static const Slang::Guid IID_IDescriptorSetLayout;
    static const Slang::Guid IID_IDescriptorSet;
    static const Slang::Guid IID_IShaderProgram;
    static const Slang::Guid IID_IPipelineLayout;
    static const Slang::Guid IID_IPipelineState;
    static const Slang::Guid IID_IResourceView;
    static const Slang::Guid IID_IFramebuffer;
    static const Slang::Guid IID_IFramebufferLayout;
    static const Slang::Guid IID_ISwapchain;
    static const Slang::Guid IID_ISamplerState;
    static const Slang::Guid IID_IResource;
    static const Slang::Guid IID_IBufferResource;
    static const Slang::Guid IID_ITextureResource;
    static const Slang::Guid IID_IInputLayout;
    static const Slang::Guid IID_IRenderer;
    static const Slang::Guid IID_IShaderObjectLayout;
    static const Slang::Guid IID_IShaderObject;
    static const Slang::Guid IID_IRenderPassLayout;
    static const Slang::Guid IID_ICommandEncoder;
    static const Slang::Guid IID_IRenderCommandEncoder;
    static const Slang::Guid IID_IComputeCommandEncoder;
    static const Slang::Guid IID_IResourceCommandEncoder;
    static const Slang::Guid IID_ICommandBuffer;
    static const Slang::Guid IID_ICommandQueue;
};

gfx::StageType translateStage(SlangStage slangStage);

class Resource : public Slang::RefObject
{
public:
    /// Get the type
    SLANG_FORCE_INLINE IResource::Type getType() const { return m_type; }
    /// True if it's a texture derived type
    SLANG_FORCE_INLINE bool isTexture() const { return int(m_type) >= int(IResource::Type::Texture1D); }
    /// True if it's a buffer derived type
    SLANG_FORCE_INLINE bool isBuffer() const { return m_type == IResource::Type::Buffer; }
protected:
    Resource(IResource::Type type)
        : m_type(type)
    {}

    IResource::Type m_type;
};

class BufferResource : public IBufferResource, public Resource
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IResource* getInterface(const Slang::Guid& guid);

public:
    typedef Resource Parent;

    /// Ctor
    BufferResource(const Desc& desc)
        : Parent(Type::Buffer)
        , m_desc(desc)
    {}

    virtual SLANG_NO_THROW IResource::Type SLANG_MCALL getType() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IBufferResource::Desc* SLANG_MCALL getDesc() SLANG_OVERRIDE;

protected:
    Desc m_desc;
};

class TextureResource : public ITextureResource, public Resource
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IResource* getInterface(const Slang::Guid& guid);

public:
    typedef Resource Parent;

    /// Ctor
    TextureResource(const Desc& desc)
        : Parent(desc.type)
        , m_desc(desc)
    {}

    virtual SLANG_NO_THROW IResource::Type SLANG_MCALL getType() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ITextureResource::Desc* SLANG_MCALL getDesc() SLANG_OVERRIDE;

protected:
    Desc m_desc;
};

Result createProgramFromSlang(IRenderer* renderer, IShaderProgram::Desc const& desc, IShaderProgram** outProgram);

class RendererBase;

typedef uint32_t ShaderComponentID;
const ShaderComponentID kInvalidComponentID = 0xFFFFFFFF;

struct ExtendedShaderObjectType
{
    slang::TypeReflection* slangType;
    ShaderComponentID componentID;
};

struct ExtendedShaderObjectTypeList
{
    Slang::ShortList<ShaderComponentID, 16> componentIDs;
    Slang::ShortList<slang::SpecializationArg, 16> components;
    void add(const ExtendedShaderObjectType& component)
    {
        componentIDs.add(component.componentID);
        components.add(slang::SpecializationArg{ slang::SpecializationArg::Kind::Type, component.slangType });
    }
    ExtendedShaderObjectType operator[](Slang::Index index) const
    {
        ExtendedShaderObjectType result;
        result.componentID = componentIDs[index];
        result.slangType = components[index].type;
        return result;
    }
    void clear()
    {
        componentIDs.clear();
        components.clear();
    }
    Slang::Index getCount()
    {
        return componentIDs.getCount();
    }
};

class ShaderObjectLayoutBase : public Slang::RefObject
{
protected:
    RendererBase* m_renderer;
    slang::TypeLayoutReflection* m_elementTypeLayout = nullptr;
    ShaderComponentID m_componentID = 0;

public:
    RendererBase* getRenderer() { return m_renderer; }

    slang::TypeLayoutReflection* getElementTypeLayout()
    {
        return m_elementTypeLayout;
    }

    ShaderComponentID getComponentID()
    {
        return m_componentID;
    }

    void initBase(RendererBase* renderer, slang::TypeLayoutReflection* elementTypeLayout);
};

class ShaderObjectBase : public IShaderObject, public Slang::RefObject
{
protected:
    // The shader object layout used to create this shader object.
    Slang::RefPtr<ShaderObjectLayoutBase> m_layout = nullptr;

    // The specialized shader object type.
    ExtendedShaderObjectType shaderObjectType = { nullptr, kInvalidComponentID };

    static bool _doesValueFitInExistentialPayload(
        slang::TypeLayoutReflection*    concreteTypeLayout,
        slang::TypeLayoutReflection*    existentialFieldLayout);

    Result _getSpecializedShaderObjectType(ExtendedShaderObjectType* outType);

public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IShaderObject* getInterface(const Slang::Guid& guid);

public:
    ShaderComponentID getComponentID()
    {
        return shaderObjectType.componentID;
    }

    // Get the final type this shader object represents. If the shader object's type has existential fields,
    // this function will return a specialized type using the bound sub-objects' type as specialization argument.
    virtual Result getSpecializedShaderObjectType(ExtendedShaderObjectType* outType);

    RendererBase* getRenderer() { return m_layout->getRenderer(); }

    SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

    SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
        SLANG_OVERRIDE
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }

    ShaderObjectLayoutBase* getLayout()
    {
        return m_layout;
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() SLANG_OVERRIDE
    {
        return m_layout->getElementTypeLayout();
    }

    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) = 0;
};

class ShaderProgramBase : public IShaderProgram, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    IShaderProgram* getInterface(const Slang::Guid& guid);

    ComPtr<slang::IComponentType> slangProgram;
};

class PipelineStateBase : public IPipelineState, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    IPipelineState* getInterface(const Slang::Guid& guid);

    struct PipelineStateDesc
    {
        PipelineType type;
        GraphicsPipelineStateDesc graphics;
        ComputePipelineStateDesc compute;
        ShaderProgramBase* getProgram()
        {
            return static_cast<ShaderProgramBase*>(type == PipelineType::Compute ? compute.program : graphics.program);
        }
    } desc;

    // The pipeline state from which this pipeline state is specialized.
    // If null, this pipeline is either an unspecialized pipeline.
    Slang::RefPtr<PipelineStateBase> unspecializedPipelineState = nullptr;

    // Indicates whether this is a specializable pipeline. A specializable
    // pipeline cannot be used directly and must be specialized first.
    bool isSpecializable = false;
    ComPtr<IShaderProgram> m_program;

    ComPtr<IPipelineLayout> m_pipelineLayout;

protected:
    void initializeBase(const PipelineStateDesc& inDesc);
};

class ShaderBinary : public Slang::RefObject
{
public:
    Slang::List<uint8_t> source;
    StageType stage;
    Slang::String entryPointName;
    Result loadFromBlob(ISlangBlob* blob);
    Result writeToBlob(ISlangBlob** outBlob);
};

struct ComponentKey
{
    Slang::UnownedStringSlice typeName;
    Slang::ShortList<ShaderComponentID> specializationArgs;
    Slang::HashCode hash;
    Slang::HashCode getHashCode()
    {
        return hash;
    }
    void updateHash()
    {
        hash = typeName.getHashCode();
        for (auto& arg : specializationArgs)
            hash = Slang::combineHash(hash, arg);
    }
};

struct PipelineKey
{
    PipelineStateBase* pipeline;
    Slang::ShortList<ShaderComponentID> specializationArgs;
    Slang::HashCode hash;
    Slang::HashCode getHashCode()
    {
        return hash;
    }
    void updateHash()
    {
        hash = Slang::getHashCode(pipeline);
        for (auto& arg : specializationArgs)
            hash = Slang::combineHash(hash, arg);
    }
    bool operator==(const PipelineKey& other)
    {
        if (pipeline != other.pipeline)
            return false;
        if (specializationArgs.getCount() != other.specializationArgs.getCount())
            return false;
        for (Slang::Index i = 0; i < other.specializationArgs.getCount(); i++)
        {
            if (specializationArgs[i] != other.specializationArgs[i])
                return false;
        }
        return true;
    }
};

struct OwningComponentKey
{
    Slang::String typeName;
    Slang::ShortList<ShaderComponentID> specializationArgs;
    Slang::HashCode hash;
    Slang::HashCode getHashCode()
    {
        return hash;
    }
    template<typename KeyType>
    bool operator==(const KeyType& other)
    {
        if (typeName != other.typeName)
            return false;
        if (specializationArgs.getCount() != other.specializationArgs.getCount())
            return false;
        for (Slang::Index i = 0; i < other.specializationArgs.getCount(); i++)
        {
            if (specializationArgs[i] != other.specializationArgs[i])
                return false;
        }
        return true;
    }
};

// A cache from specialization keys to a specialized `ShaderKernel`.
class ShaderCache : public Slang::RefObject
{
public:
    ShaderComponentID getComponentId(slang::TypeReflection* type);
    ShaderComponentID getComponentId(Slang::UnownedStringSlice name);
    ShaderComponentID getComponentId(ComponentKey key);

    Slang::ComPtr<IPipelineState> getSpecializedPipelineState(PipelineKey programKey)
    {
        Slang::ComPtr<IPipelineState> result;
        if (specializedPipelines.TryGetValue(programKey, result))
            return result;
        return nullptr;
    }
    void addSpecializedPipeline(PipelineKey key, Slang::ComPtr<IPipelineState> specializedPipeline);
    void free()
    {
        specializedPipelines = decltype(specializedPipelines)();
        componentIds = decltype(componentIds)();
    }

protected:
    Slang::OrderedDictionary<OwningComponentKey, ShaderComponentID> componentIds;
    Slang::OrderedDictionary<PipelineKey, Slang::ComPtr<IPipelineState>> specializedPipelines;
};

// Renderer implementation shared by all platforms.
// Responsible for shader compilation, specialization and caching.
class RendererBase : public Slang::RefObject, public IRenderer
{
    friend class ShaderObjectBase;
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(
        const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* featureName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSlangSession(slang::ISession** outSlangSession) SLANG_OVERRIDE;
    IRenderer* getInterface(const Slang::Guid& guid);

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(slang::TypeReflection* type, IShaderObject** outObject) SLANG_OVERRIDE;

    Result getShaderObjectLayout(
        slang::TypeReflection*      type,
        ShaderObjectLayoutBase**    outLayout);

public:
    ExtendedShaderObjectTypeList specializationArgs;
    // Given current pipeline and root shader object binding, generate and bind a specialized pipeline if necessary.
    Result maybeSpecializePipeline(
        PipelineStateBase* currentPipeline,
        ShaderObjectBase* rootObject,
        Slang::RefPtr<PipelineStateBase>& outNewPipeline);


    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) = 0;

    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout,
        IShaderObject**         outObject) = 0;


protected:
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc);
protected:
    Slang::List<Slang::String> m_features;
public:
    SlangContext slangContext;
    ShaderCache shaderCache;

    Slang::Dictionary<slang::TypeReflection*, Slang::RefPtr<ShaderObjectLayoutBase>> m_shaderObjectLayoutCache;
};

}
