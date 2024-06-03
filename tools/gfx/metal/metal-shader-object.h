// metal-shader-object.h
#pragma once

#include "metal-resource-views.h"
#include "metal-sampler.h"
#include "metal-shader-object-layout.h"

namespace gfx
{

namespace metal
{

struct CombinedTextureSamplerSlot
{
    RefPtr<TextureResourceViewImpl> textureView;
    RefPtr<SamplerStateImpl> sampler;
    operator bool() { return textureView && sampler; }
};

class ShaderObjectImpl
    : public ShaderObjectBaseImpl<ShaderObjectImpl, ShaderObjectLayoutImpl, SimpleShaderObjectData>
{
public:
    static Result create(
        IDevice* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject);

    RendererBase* getDevice();

    virtual SLANG_NO_THROW GfxCount SLANG_MCALL getEntryPointCount() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override;

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override;

    virtual SLANG_NO_THROW Size SLANG_MCALL getSize() override;

    // TODO: Changed size_t to Size? inSize assigned to an Index variable inside implementation
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& inOffset, void const* data, size_t inSize) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) override;

protected:
    friend class RootShaderObjectLayout;

    Result init(IDevice* device, ShaderObjectLayoutImpl* layout);

public:
};

class MutableShaderObjectImpl
    : public MutableShaderObject<MutableShaderObjectImpl, ShaderObjectLayoutImpl>
{
public:
};

class EntryPointShaderObject : public ShaderObjectImpl
{
    typedef ShaderObjectImpl Super;

public:
    static Result create(
        IDevice* device, EntryPointLayout* layout, EntryPointShaderObject** outShaderObject);

    EntryPointLayout* getLayout();

protected:
    Result init(IDevice* device, EntryPointLayout* layout);
};


class RootShaderObjectImpl : public ShaderObjectImpl
{
    using Super = ShaderObjectImpl;

public:
    // Override default reference counting behavior to disable lifetime management.
    // Root objects are managed by command buffer and does not need to be freed by the user.
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

public:
    RootShaderObjectLayout* getLayout();

    RootShaderObjectLayout* getSpecializedLayout();

    List<RefPtr<EntryPointShaderObject>> const& getEntryPoints() const;

    virtual GfxCount SLANG_MCALL getEntryPointCount() override;
    virtual Result SLANG_MCALL getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap) override;

#if 0
    /// Bind this object as a root shader object
    Result bindAsRoot(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        RootShaderObjectLayout* layout);
#endif

    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override;

public:
    Result init(IDevice* device, RootShaderObjectLayout* layout);
    List<RefPtr<EntryPointShaderObject>> m_entryPoints;
};


} // namespace metal
} // namespace gfx
