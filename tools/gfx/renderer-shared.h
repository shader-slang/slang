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
    static const Slang::Guid IID_IRayTracingCommandEncoder;
    static const Slang::Guid IID_ICommandBuffer;
    static const Slang::Guid IID_ICommandQueue;
    static const Slang::Guid IID_IQueryPool;
    static const Slang::Guid IID_IAccelerationStructure;
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

class ResourceViewBase
    : public IResourceView
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IResourceView* getInterface(const Slang::Guid& guid);
};

class AccelerationStructureBase
    : public IAccelerationStructure
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IAccelerationStructure* getInterface(const Slang::Guid& guid);
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
    void addRange(const ExtendedShaderObjectTypeList& list)
    {
        for (Slang::Index i = 0; i < list.getCount(); i++)
        {
            add(list[i]);
        }
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
    Slang::Index getCount() const
    {
        return componentIDs.getCount();
    }
};

struct ExtendedShaderObjectTypeListObject
    : public ExtendedShaderObjectTypeList
    , public Slang::RefObject
{};

class ShaderObjectLayoutBase : public Slang::RefObject
{
protected:
    // We always use a weak reference to the `IDevice` object here.
    // `ShaderObject` implementations will make sure to hold a strong reference to `IDevice`
    // while a `ShaderObjectLayout` may still be used.
    RendererBase* m_renderer;
    slang::TypeLayoutReflection* m_elementTypeLayout = nullptr;
    ShaderComponentID m_componentID = 0;

    /// The container type of this shader object. When `m_containerType` is `StructuredBuffer` or
    /// `UnsizedArray`, this shader object represents a collection instead of a single object.
    ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

public:
    ShaderObjectContainerType getContainerType() { return m_containerType; }

    static slang::TypeLayoutReflection* _unwrapParameterGroups(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectContainerType& outContainerType)
    {
        outContainerType = ShaderObjectContainerType::None;
        for (;;)
        {
            if (!typeLayout->getType())
            {
                if (auto elementTypeLayout = typeLayout->getElementTypeLayout())
                    typeLayout = elementTypeLayout;
            }
            switch (typeLayout->getKind())
            {
            case slang::TypeReflection::Kind::Array:
                SLANG_ASSERT(outContainerType == ShaderObjectContainerType::None);
                outContainerType = ShaderObjectContainerType::Array;
                typeLayout = typeLayout->getElementTypeLayout();
                return typeLayout;
            case slang::TypeReflection::Kind::Resource:
                {
                    if (typeLayout->getResourceShape() != SLANG_STRUCTURED_BUFFER)
                        break;
                    SLANG_ASSERT(outContainerType == ShaderObjectContainerType::None);
                    outContainerType = ShaderObjectContainerType::StructuredBuffer;
                    typeLayout = typeLayout->getElementTypeLayout();
                }
                return typeLayout;
            default:
                break;
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

class SimpleShaderObjectData
{
public:
    // Any "ordinary" / uniform data for this object
    Slang::List<char> m_ordinaryData;
    // The structured buffer resource used when the object represents a structured buffer.
    Slang::RefPtr<BufferResource> m_structuredBuffer;
    // The structured buffer resource view used when the object represents a structured buffer.
    Slang::RefPtr<ResourceViewBase> m_structuredBufferView;
    Slang::RefPtr<ResourceViewBase> m_rwStructuredBufferView;

    Slang::Index getCount() { return m_ordinaryData.getCount(); }
    void setCount(Slang::Index count) { m_ordinaryData.setCount(count); }
    char* getBuffer() { return m_ordinaryData.getBuffer(); }

    /// Returns a StructuredBuffer resource view for GPU access into the buffer content.
    /// Creates a StructuredBuffer resource if it has not been created.
    ResourceViewBase* getResourceView(
        RendererBase* device,
        slang::TypeLayoutReflection* elementLayout,
        slang::BindingType bindingType);
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

    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) = 0;

    RendererBase* getRenderer() { return m_layout->getDevice(); }

    SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

    SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
        SLANG_OVERRIDE
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }

    ShaderObjectLayoutBase* getLayoutBase() { return m_layout; }

    SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() SLANG_OVERRIDE
    {
        return m_layout->getElementTypeLayout();
    }

    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() SLANG_OVERRIDE
    {
        return m_layout->getContainerType();
    }

        /// Sets the RTTI ID and RTTI witness table fields of an existential value.
    Result setExistentialHeader(
        slang::TypeReflection* existentialType,
        slang::TypeReflection* concreteType,
        ShaderOffset offset);
};

template<typename TShaderObjectImpl, typename TShaderObjectLayoutImpl, typename TShaderObjectData>
class ShaderObjectBaseImpl : public ShaderObjectBase
{
protected:
    TShaderObjectData m_data;
    Slang::List<Slang::RefPtr<TShaderObjectImpl>> m_objects;
    Slang::List<Slang::RefPtr<ExtendedShaderObjectTypeListObject>> m_userProvidedSpecializationArgs;

    // Specialization args for a StructuredBuffer object.
    ExtendedShaderObjectTypeList m_structuredBufferSpecializationArgs;

public:
    TShaderObjectLayoutImpl* getLayout()
    {
        return static_cast<TShaderObjectLayoutImpl*>(m_layout.Ptr());
    }

    void* getBuffer() { return m_data.getBuffer(); }
    size_t getBufferSize() { return (size_t)m_data.getCount(); }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** outObject) SLANG_OVERRIDE
    {
        SLANG_ASSERT(outObject);
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

        returnComPtr(outObject, m_objects[bindingRange.subObjectIndex + offset.bindingArrayIndex]);
        return SLANG_OK;
    }

    void setSpecializationArgsForContainerElement(ExtendedShaderObjectTypeList& specializationArgs)
    {
        // Compute specialization args for the structured buffer object.
        // If we haven't filled anything to `m_structuredBufferSpecializationArgs` yet,
        // use `specializationArgs` directly.
        if (m_structuredBufferSpecializationArgs.getCount() == 0)
        {
            m_structuredBufferSpecializationArgs = Slang::_Move(specializationArgs);
        }
        else
        {
            // If `m_structuredBufferSpecializationArgs` already contains some arguments, we
            // need to check if they are the same as `specializationArgs`, and replace
            // anything that is different with `__Dynamic` because we cannot specialize the
            // buffer type if the element types are not the same.
            SLANG_ASSERT(
                m_structuredBufferSpecializationArgs.getCount() == specializationArgs.getCount());
            auto device = getRenderer();
            for (Slang::Index i = 0; i < m_structuredBufferSpecializationArgs.getCount(); i++)
            {
                if (m_structuredBufferSpecializationArgs[i].componentID !=
                    specializationArgs[i].componentID)
                {
                    auto dynamicType = device->slangContext.session->getDynamicType();
                    m_structuredBufferSpecializationArgs.componentIDs[i] =
                        device->shaderCache.getComponentId(dynamicType);
                    m_structuredBufferSpecializationArgs.components[i] =
                        slang::SpecializationArg::fromType(dynamicType);
                }
            }
        }
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object) SLANG_OVERRIDE
    {
        auto layout = getLayout();
        auto subObject = static_cast<TShaderObjectImpl*>(object);
        // There are three different cases in `setObject`.
        // 1. `this` object represents a StructuredBuffer, and `object` is an
        //    element to be written into the StructuredBuffer.
        // 2. `object` represents a StructuredBuffer and we are setting it into
        //    a StructuredBuffer typed field in `this` object.
        // 3. We are setting `object` as an ordinary sub-object, e.g. an existential
        //    field, a constant buffer or a parameter block.
        // We handle each case separately below.

        if (layout->getContainerType() != ShaderObjectContainerType::None)
        {
            // Case 1:
            // We are setting an element into a `StructuredBuffer` object.
            // We need to hold a reference to the element object, as well as
            // writing uniform data to the plain buffer.
            if (offset.bindingArrayIndex >= m_objects.getCount())
            {
                m_objects.setCount(offset.bindingArrayIndex + 1);
                auto stride = layout->getElementTypeLayout()->getStride();
                m_data.setCount(m_objects.getCount() * stride);
            }
            m_objects[offset.bindingArrayIndex] = subObject;

            ExtendedShaderObjectTypeList specializationArgs;

            auto payloadOffset = offset;

            // If the element type of the StructuredBuffer field is an existential type,
            // we need to make sure to fill in the existential value header (RTTI ID and
            // witness table IDs).
            if (layout->getElementTypeLayout()->getKind() == slang::TypeReflection::Kind::Interface)
            {
                auto existentialType = layout->getElementTypeLayout()->getType();
                ExtendedShaderObjectType concreteType;
                SLANG_RETURN_ON_FAIL(subObject->getSpecializedShaderObjectType(&concreteType));
                SLANG_RETURN_ON_FAIL(
                    setExistentialHeader(existentialType, concreteType.slangType, offset));
                payloadOffset.uniformOffset += 16;

                // If this object is a `StructuredBuffer<ISomeInterface>`, then the
                // specialization argument should be the specialized type of the sub object
                // itself.
                specializationArgs.add(concreteType);
            }
            else
            {
                // If this object is a `StructuredBuffer<SomeConcreteType>`, then the
                // specialization
                // argument should come recursively from the sub object.
                subObject->collectSpecializationArgs(specializationArgs);
            }
            SLANG_RETURN_ON_FAIL(setData(
                payloadOffset,
                subObject->m_data.getBuffer(),
                (size_t)subObject->m_data.getCount()));

            setSpecializationArgsForContainerElement(specializationArgs);
            return SLANG_OK;
        }

        // Case 2 & 3, setting object as an StructuredBuffer, ConstantBuffer, ParameterBlock or
        // existential value.

        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;

        auto bindingRangeIndex = offset.bindingRangeIndex;
        auto bindingRange = layout->getBindingRange(bindingRangeIndex);

        m_objects[bindingRange.subObjectIndex + offset.bindingArrayIndex] = subObject;

        switch (bindingRange.bindingType)
        {
        case slang::BindingType::ExistentialValue:
            {
                // If the range being assigned into represents an interface/existential-type
                // leaf field, then we need to consider how the `object` being assigned here
                // affects specialization. We may also need to assign some data from the
                // sub-object into the ordinary data buffer for the parent object.
                //
                // A leaf field of interface type is laid out inside of the parent object
                // as a tuple of `(RTTI, WitnessTable, Payload)`. The layout of these fields
                // is a contract between the compiler and any runtime system, so we will
                // need to rely on details of the binary layout.

                // We start by querying the layout/type of the concrete value that the
                // application is trying to store into the field, and also the layout/type of
                // the leaf existential-type field itself.
                //
                auto concreteTypeLayout = subObject->getElementTypeLayout();
                auto concreteType = concreteTypeLayout->getType();
                //
                auto existentialTypeLayout =
                    layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(
                        bindingRangeIndex);
                auto existentialType = existentialTypeLayout->getType();

                // Fills in the first and second field of the tuple that specify RTTI type ID
                // and witness table ID.
                SLANG_RETURN_ON_FAIL(setExistentialHeader(existentialType, concreteType, offset));

                // The third field of the tuple (offset 16) is the "payload" that is supposed to
                // hold the data for a value of the given concrete type.
                //
                auto payloadOffset = offset;
                payloadOffset.uniformOffset += 16;

                // There are two cases we need to consider here for how the payload might be
                // used:
                //
                // * If the concrete type of the value being bound is one that can "fit" into
                // the
                //   available payload space,  then it should be stored in the payload.
                //
                // * If the concrete type of the value cannot fit in the payload space, then it
                //   will need to be stored somewhere else.
                //
                if (_doesValueFitInExistentialPayload(concreteTypeLayout, existentialTypeLayout))
                {
                    // If the value can fit in the payload area, then we will go ahead and copy
                    // its bytes into that area.
                    //
                    setData(
                        payloadOffset, subObject->m_data.getBuffer(), subObject->m_data.getCount());
                }
                else
                {
                    // If the value does *not *fit in the payload area, then there is nothing
                    // we can do at this point (beyond saving a reference to the sub-object,
                    // which was handled above).
                    //
                    // Once all the sub-objects have been set into the parent object, we can
                    // compute a specialized layout for it, and that specialized layout can tell
                    // us where the data for these sub-objects has been laid out.
                    return SLANG_E_NOT_IMPLEMENTED;
                }
            }
            break;
        case slang::BindingType::MutableRawBuffer:
        case slang::BindingType::RawBuffer:
            {
                // If we are setting into a `StructuredBuffer` field, make sure we create and set
                // the StructuredBuffer resource as well.
                setResource(
                    offset,
                    subObject->m_data.getResourceView(
                        getRenderer(),
                        subObject->getElementTypeLayout(),
                        bindingRange.bindingType));
            }
            break;
        }
        return SLANG_OK;
    }

    Result getExtendedShaderTypeListFromSpecializationArgs(
        ExtendedShaderObjectTypeList& list,
        const slang::SpecializationArg* args,
        uint32_t count)
    {
        auto device = getRenderer();
        for (uint32_t i = 0; i < count; i++)
        {
            gfx::ExtendedShaderObjectType extendedType;
            switch (args[i].kind)
            {
            case slang::SpecializationArg::Kind::Type:
                extendedType.slangType = args[i].type;
                extendedType.componentID = device->shaderCache.getComponentId(args[i].type);
                break;
            default:
                SLANG_ASSERT(false && "Unexpected specialization argument kind.");
                return SLANG_FAIL;
            }
            list.add(extendedType);
        }
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(
        ShaderOffset const& offset,
        const slang::SpecializationArg* args,
        uint32_t count) override
    {
        auto layout = getLayout();

        // If the shader object is a container, delegate the processing to
        // `setSpecializationArgsForContainerElements`.
        if (layout->getContainerType() != ShaderObjectContainerType::None)
        {
            ExtendedShaderObjectTypeList argList;
            SLANG_RETURN_ON_FAIL(
                getExtendedShaderTypeListFromSpecializationArgs(argList, args, count));
            setSpecializationArgsForContainerElement(argList);
            return SLANG_OK;
        }

        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;

        auto bindingRangeIndex = offset.bindingRangeIndex;
        auto bindingRange = layout->getBindingRange(bindingRangeIndex);
        auto objectIndex = bindingRange.subObjectIndex + offset.bindingArrayIndex;
        if (objectIndex >= m_userProvidedSpecializationArgs.getCount())
            m_userProvidedSpecializationArgs.setCount(objectIndex + 1);
        if (!m_userProvidedSpecializationArgs[objectIndex])
        {
            m_userProvidedSpecializationArgs[objectIndex] =
                new ExtendedShaderObjectTypeListObject();
        }
        else
        {
            m_userProvidedSpecializationArgs[objectIndex]->clear();
        }
        SLANG_RETURN_ON_FAIL(getExtendedShaderTypeListFromSpecializationArgs(
            *m_userProvidedSpecializationArgs[objectIndex], args, count));
        return SLANG_OK;
    }

    // Appends all types that are used to specialize the element type of this shader object in
    // `args` list.
    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
    {
        if (m_layout->getContainerType() != ShaderObjectContainerType::None)
        {
            args.addRange(m_structuredBufferSpecializationArgs);
            return SLANG_OK;
        }

        auto device = getRenderer();
        auto& subObjectRanges = getLayout()->getSubObjectRanges();
        // The following logic is built on the assumption that all fields that involve
        // existential types (and therefore require specialization) will results in a sub-object
        // range in the type layout. This allows us to simply scan the sub-object ranges to find
        // out all specialization arguments.
        Slang::Index subObjectRangeCount = subObjectRanges.getCount();

        for (Slang::Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
             subObjectRangeIndex++)
        {
            auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
            auto const& bindingRange =
                getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);

            Slang::Index oldArgsCount = args.getCount();

            Slang::Index count = bindingRange.count;

            for (Slang::Index subObjectIndexInRange = 0; subObjectIndexInRange < count;
                 subObjectIndexInRange++)
            {
                ExtendedShaderObjectTypeList typeArgs;
                auto objectIndex = bindingRange.subObjectIndex + subObjectIndexInRange;
                auto subObject = m_objects[objectIndex];

                if (!subObject)
                    continue;

                if (objectIndex < m_userProvidedSpecializationArgs.getCount() &&
                    m_userProvidedSpecializationArgs[objectIndex])
                {
                    args.addRange(*m_userProvidedSpecializationArgs[objectIndex]);
                    continue;
                }

                switch (bindingRange.bindingType)
                {
                case slang::BindingType::ExistentialValue:
                    {
                        // A binding type of `ExistentialValue` means the sub-object represents a
                        // interface-typed field. In this case the specialization argument for this
                        // field is the actual specialized type of the bound shader object. If the
                        // shader object's type is an ordinary type without existential fields, then
                        // the type argument will simply be the ordinary type. But if the sub
                        // object's type is itself a specialized type, we need to make sure to use
                        // that type as the specialization argument.

                        ExtendedShaderObjectType specializedSubObjType;
                        SLANG_RETURN_ON_FAIL(
                            subObject->getSpecializedShaderObjectType(&specializedSubObjType));
                        typeArgs.add(specializedSubObjType);
                        break;
                    }
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ConstantBuffer:
                case slang::BindingType::RawBuffer:
                case slang::BindingType::MutableRawBuffer:
                    // Currently we only handle the case where the field's type is
                    // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where
                    // `SomeStruct` is a struct type (not directly an interface type). In this case,
                    // we just recursively collect the specialization arguments from the bound sub
                    // object.
                    SLANG_RETURN_ON_FAIL(subObject->collectSpecializationArgs(typeArgs));
                    // TODO: we need to handle the case where the field is of the form
                    // `ParameterBlock<IFoo>`. We should treat this case the same way as the
                    // `ExistentialValue` case here, but currently we lack a mechanism to
                    // distinguish the two scenarios.
                    break;
                }

                auto addedTypeArgCountForCurrentRange = args.getCount() - oldArgsCount;
                if (addedTypeArgCountForCurrentRange == 0)
                {
                    args.addRange(typeArgs);
                }
                else
                {
                    // If type arguments for each elements in the array is different, use
                    // `__Dynamic` type for the differing argument to disable specialization.
                    SLANG_ASSERT(addedTypeArgCountForCurrentRange == typeArgs.getCount());
                    for (Slang::Index i = 0; i < addedTypeArgCountForCurrentRange; i++)
                    {
                        if (args[i + oldArgsCount].componentID != typeArgs[i].componentID)
                        {
                            auto dynamicType = device->slangContext.session->getDynamicType();
                            args.componentIDs[i + oldArgsCount] =
                                device->shaderCache.getComponentId(dynamicType);
                            args.components[i + oldArgsCount] =
                                slang::SpecializationArg::fromType(dynamicType);
                        }
                    }
                }
            }
        }
        return SLANG_OK;
    }
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

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType containerType,
        IShaderObject** outObject) SLANG_OVERRIDE;

    // Provides a default implementation that returns SLANG_E_NOT_AVAILABLE for platforms
    // without ray tracing support.
    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;

    // Provides a default implementation that returns SLANG_E_NOT_AVAILABLE for platforms
    // without ray tracing support.
    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) override;

    Result getShaderObjectLayout(
        slang::TypeReflection*      type,
        ShaderObjectContainerType   container,
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
