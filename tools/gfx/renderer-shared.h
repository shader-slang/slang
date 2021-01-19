#pragma once

#include "tools/gfx/render.h"

#include "core/slang-smart-pointer.h"

namespace gfx
{

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

}
