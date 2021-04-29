#pragma once

#include "slang-gfx.h"
#include "slang-context.h"
#include "core/slang-basic.h"
#include "core/slang-com-object.h"

#include "resource-desc-utils.h"

namespace gfx
{

struct GfxGUID
{
    static const Slang::Guid IID_ISlangUnknown;
    static const Slang::Guid IID_IShaderProgram;
    static const Slang::Guid IID_ITransientResourceHeap;
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
    static const Slang::Guid IID_IDevice;
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

// We use a `BreakableReference` to avoid the cyclic reference situation in gfx implementation.
// It is a common scenario where objects created from an `IDevice` implementation needs to hold
// a strong reference to the device object that creates them. For example, a `Buffer` or a
// `CommandQueue` needs to store a `m_device` member that points to the `IDevice`. At the same
// time, the device implementation may also hold a reference to some of the objects it created
// to represent the current device/binding state. Both parties would like to maintain a strong
// reference to each other to achieve robustness against arbitrary ordering of destruction that
// can be triggered by the user. However this creates cyclic reference situations that break
// the `RefPtr` recyling mechanism. To solve this problem, we instead make each object reference
// the device via a `BreakableReference<TDeviceImpl>` pointer. A breakable reference can be
// turned into a weak reference via its `breakStrongReference()` call.
// If we know there is a cyclic reference between an API object and the device/pool that creates it,
// we can break the cycle when there is no longer any public references that come from `ComPtr`s to
// the API object, by turning the reference to the device object from the API object to a weak
// reference. 
// The following example illustrate how this mechanism works:
// Suppose we have
// ```
// class DeviceImpl : IDevice { RefPtr<ShaderObject> m_currentObject; };
// class ShaderObjectImpl : IShaderObject { BreakableReference<DeviceImpl> m_device; };
// ```
// And the user creates a device and a shader object, then somehow having the device reference
// the shader object (this may not happen in actual implemetations, we just use it to illustrate
// the situation):
// ```
// ComPtr<IDevice> device = createDevice();
// ComPtr<ISomeResource> res = device->createResourceX(...);
// device->m_currentResource = res;
// ```
// This setup is robust to any destruction ordering. If user releases reference to `device` first,
// then the device object will not be freed yet, since there is still a strong reference to the device
// implementation via `res->m_device`. Next when the user releases reference to `res`, the public
// reference count to `res` via `ComPtr`s will go to 0, therefore triggering the call to
// `res->m_device.breakStrongReference()`, releasing the remaining reference to device. This will cause
// `device` to start destruction, which will release its strong reference to `res` during execution of
// its destructor. Finally, this will triger the actual destruction of `res`.
// On the other hand, if the user releases reference to `res` first, then the strong reference to `device`
// will be broken immediately, but the actual destruction of `res` will not start. Next when the user
// releases `device`, there will no longer be any other references to `device`, so the destruction of
// `device` will start, causing the release of the internal reference to `res`, leading to its destruction.
// Note that the above logic only works if it is known that there is a cyclic reference. If there are no
// such cyclic reference, then it will be incorrect to break the strong reference to `IDevice` upon
// public reference counter dropping to 0. This is because the actual destructor of `res` take place
// after breaking the cycle, but if the resource's strong reference to the device is already the last reference,
// turning that reference to weak reference will immediately trigger destruction of `device`, after which
// we can no longer destruct `res` if the destructor needs `device`. Therefore we need to be careful
// when using `BreakableReference`, and make sure we only call `breakStrongReference` only when it is known
// that there is a cyclic reference. Luckily for all scenarios so far this is statically known.
template<typename T>
class BreakableReference
{
private:
    Slang::RefPtr<T> m_strongPtr;
    T* m_weakPtr = nullptr;

public:
    BreakableReference() = default;

    BreakableReference(T* p) { *this = p; }

    BreakableReference(Slang::RefPtr<T> const& p) { *this = p; }

    void setWeakReference(T* p) { m_weakPtr = p;  m_strongPtr = nullptr; }

    T& operator*() const { return *get(); }

    T* operator->() const { return get(); }

    T* get() const { return m_weakPtr; }

    operator T*() const { return get(); }

    void operator=(Slang::RefPtr<T> const& p)
    {
        m_strongPtr = p;
        m_weakPtr = p.Ptr();
    }

    void operator=(T* p)
    {
        m_strongPtr = p;
        m_weakPtr = p;
    }

    void breakStrongReference() { m_strongPtr = nullptr; }

    void establishStrongReference() { m_strongPtr = m_weakPtr; }
};

// Helpers for returning an object implementation as COM pointer.
template<typename TInterface, typename TImpl>
void returnComPtr(TInterface** outInterface, TImpl* rawPtr)
{
    static_assert(
        !std::is_base_of<Slang::RefObject, TInterface>::value,
        "TInterface must be an interface type.");
    rawPtr->addRef();
    *outInterface = rawPtr;
}

template <typename TInterface, typename TImpl>
void returnComPtr(TInterface** outInterface, const Slang::RefPtr<TImpl>& refPtr)
{
    static_assert(
        !std::is_base_of<Slang::RefObject, TInterface>::value,
        "TInterface must be an interface type.");
    refPtr->addRef();
    *outInterface = refPtr.Ptr();
}

template <typename TInterface, typename TImpl>
void returnComPtr(TInterface** outInterface, Slang::ComPtr<TImpl>& comPtr)
{
    static_assert(
        !std::is_base_of<Slang::RefObject, TInterface>::value,
        "TInterface must be an interface type.");
    *outInterface = comPtr.detach();
}

// Helpers for returning an object implementation as RefPtr.
template <typename TDest, typename TImpl>
void returnRefPtr(TDest** outPtr, Slang::RefPtr<TImpl>& refPtr)
{
    static_assert(
        std::is_base_of<Slang::RefObject, TDest>::value, "TDest must be a non-interface type.");
    static_assert(
        std::is_base_of<Slang::RefObject, TImpl>::value, "TImpl must be a non-interface type.");
    *outPtr = refPtr.Ptr();
    refPtr->addReference();
}

template <typename TDest, typename TImpl>
void returnRefPtrMove(TDest** outPtr, Slang::RefPtr<TImpl>& refPtr)
{
    static_assert(
        std::is_base_of<Slang::RefObject, TDest>::value, "TDest must be a non-interface type.");
    static_assert(
        std::is_base_of<Slang::RefObject, TImpl>::value, "TImpl must be a non-interface type.");
    *outPtr = refPtr.detach();
}


gfx::StageType translateStage(SlangStage slangStage);

class Resource : public Slang::ComObject
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
    SLANG_COM_OBJECT_IUNKNOWN_ALL
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
    SLANG_COM_OBJECT_IUNKNOWN_ALL
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
    // We always use a weak reference to the `IDevice` object here.
    // `ShaderObject` implementations will make sure to hold a strong reference to `IDevice`
    // while a `ShaderObjectLayout` may still be used.
    RendererBase* m_renderer;
    slang::TypeLayoutReflection* m_elementTypeLayout = nullptr;
    ShaderComponentID m_componentID = 0;
public:
    static slang::TypeLayoutReflection* _unwrapParameterGroups(slang::TypeLayoutReflection* typeLayout)
    {
        for (;;)
        {
            if (!typeLayout->getType())
            {
                if (auto elementTypeLayout = typeLayout->getElementTypeLayout())
                    typeLayout = elementTypeLayout;
            }

            switch (typeLayout->getKind())
            {
            default:
                return typeLayout;

            case slang::TypeReflection::Kind::ConstantBuffer:
            case slang::TypeReflection::Kind::ParameterBlock:
                typeLayout = typeLayout->getElementTypeLayout();
                continue;
            }
        }
    }


public:
    RendererBase* getDevice() { return m_renderer; }

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

class ShaderObjectBase : public IShaderObject, public Slang::ComObject
{
protected:
    // A strong reference to `IDevice` to make sure the weak device reference in
    // `ShaderObjectLayout`s are valid whenever they might be used.
    BreakableReference<RendererBase> m_device;

    // The shader object layout used to create this shader object.
    Slang::RefPtr<ShaderObjectLayoutBase> m_layout = nullptr;

    // The specialized shader object type.
    ExtendedShaderObjectType shaderObjectType = { nullptr, kInvalidComponentID };

    static bool _doesValueFitInExistentialPayload(
        slang::TypeLayoutReflection*    concreteTypeLayout,
        slang::TypeLayoutReflection*    existentialFieldLayout);

    Result _getSpecializedShaderObjectType(ExtendedShaderObjectType* outType);

public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IShaderObject* getInterface(const Slang::Guid& guid);
    void breakStrongReferenceToDevice() { m_device.breakStrongReference(); }

public:
    ShaderComponentID getComponentID()
    {
        return shaderObjectType.componentID;
    }

    // Get the final type this shader object represents. If the shader object's type has existential fields,
    // this function will return a specialized type using the bound sub-objects' type as specialization argument.
    virtual Result getSpecializedShaderObjectType(ExtendedShaderObjectType* outType);

    RendererBase* getRenderer() { return m_layout->getDevice(); }

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

class ShaderProgramBase : public IShaderProgram, public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IShaderProgram* getInterface(const Slang::Guid& guid);

    ComPtr<slang::IComponentType> slangProgram;
};

class InputLayoutBase
    : public IInputLayout
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IInputLayout* getInterface(const Slang::Guid& guid);
};

class FramebufferLayoutBase
    : public IFramebufferLayout
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IFramebufferLayout* getInterface(const Slang::Guid& guid);
};

class PipelineStateBase
    : public IPipelineState
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
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

    // We need to hold inputLayout and framebufferLayout objects alive, since we may use it to
    // create specialized pipeline states later.
    Slang::RefPtr<InputLayoutBase> inputLayout;
    Slang::RefPtr<FramebufferLayoutBase> framebufferLayout;

    // The pipeline state from which this pipeline state is specialized.
    // If null, this pipeline is either an unspecialized pipeline.
    Slang::RefPtr<PipelineStateBase> unspecializedPipelineState = nullptr;

    // Indicates whether this is a specializable pipeline. A specializable
    // pipeline cannot be used directly and must be specialized first.
    bool isSpecializable = false;
    Slang::RefPtr<ShaderProgramBase> m_program;
    template <typename TProgram> TProgram* getProgram()
    {
        return static_cast<TProgram*>(m_program.Ptr());
    }

protected:
    void initializeBase(const PipelineStateDesc& inDesc);
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

    Slang::RefPtr<PipelineStateBase> getSpecializedPipelineState(PipelineKey programKey)
    {
        Slang::RefPtr<PipelineStateBase> result;
        if (specializedPipelines.TryGetValue(programKey, result))
            return result;
        return nullptr;
    }
    void addSpecializedPipeline(
        PipelineKey key,
        Slang::RefPtr<PipelineStateBase> specializedPipeline);
    void free()
    {
        specializedPipelines = decltype(specializedPipelines)();
        componentIds = decltype(componentIds)();
    }

protected:
    Slang::OrderedDictionary<OwningComponentKey, ShaderComponentID> componentIds;
    Slang::OrderedDictionary<PipelineKey, Slang::RefPtr<PipelineStateBase>> specializedPipelines;
};

// Renderer implementation shared by all platforms.
// Responsible for shader compilation, specialization and caching.
class RendererBase : public IDevice, public Slang::ComObject
{
    friend class ShaderObjectBase;
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(
        const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* featureName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSlangSession(slang::ISession** outSlangSession) SLANG_OVERRIDE;
    IDevice* getInterface(const Slang::Guid& guid);

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

IDebugCallback*& _getDebugCallback();
IDebugCallback* _getNullDebugCallback();
inline IDebugCallback* getDebugCallback()
{
    auto rs = _getDebugCallback();
    if (rs)
    {
        return rs;
    }
    else
    {
        return _getNullDebugCallback();
    }
}
}
