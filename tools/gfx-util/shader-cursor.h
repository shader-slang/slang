#pragma once

#include "tools/gfx/render.h"

namespace gfx
{

/// Represents a "pointer" to the storage for a shader parameter of a (dynamically) known type.
///
/// A `ShaderCursor` serves as a pointer-like type for things stored inside a `ShaderObject`.
///
/// A cursor that points to the entire content of a shader object can be formed as
/// `ShaderCursor(someObject)`. A cursor pointing to a structure field or array element can be
/// formed from another cursor using `getField` or `getElement` respectively.
///
/// Given a cursor pointing to a value of some "primitive" type, we can set or get the value
/// using operations like `setResource`, `getResource`, etc.
///
/// Because type information for shader parameters is being reflected dynamically, all type
/// checking for shader cursors occurs at runtime, and errors may occur when attempting to
/// set a parameter using a value of an inappropriate type. As much as possible, `ShaderCursor`
/// attempts to protect against these cases and return an error `Result` or an invalid
/// cursor, rather than allowing operations to proceed with incorrect types.
///
struct ShaderCursor
{
    IShaderObject* m_baseObject = nullptr;
    slang::TypeLayoutReflection* m_typeLayout = nullptr;
    ShaderOffset m_offset;

    /// Get the type (layout) of the value being pointed at by the cursor
    slang::TypeLayoutReflection* getTypeLayout() const { return m_typeLayout; }

    /// Is this cursor valid (that is, does it seem to point to an actual location)?
    ///
    /// This check is equivalent to checking whether a pointer is null, so it is
    /// a very weak sense of "valid." In particular, it is possible to form a
    /// `ShaderCursor` for which `isValid()` is true, but attempting to get or
    /// set the value would be an error (like dereferencing a garbage pointer).
    ///
    bool isValid() const { return m_baseObject != nullptr; }

    Result getDereferenced(ShaderCursor& outCursor) const;

    ShaderCursor getDereferenced()
    {
        ShaderCursor result;
        getDereferenced(result);
        return result;
    }

    /// Form a cursor pointing to the field with the given `name` within the value this cursor
    /// points at.
    ///
    /// If the operation succeeds, then the field cursor is written to `outCursor`.
    Result getField(const char* nameBegin, const char* nameEnd, ShaderCursor& outCursor);

    ShaderCursor getField(const char* name)
    {
        ShaderCursor cursor;
        getField(name, nullptr, cursor);
        return cursor;
    }

    ShaderCursor getElement(SlangInt index);

    static Result followPath(const char* path, ShaderCursor& ioCursor);

    ShaderCursor getPath(const char* path)
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }

    ShaderCursor() {}

    ShaderCursor(IShaderObject* object)
        : m_baseObject(object)
        , m_typeLayout(object->getElementTypeLayout())
    {}

    SlangResult setData(void const* data, size_t size)
    {
        return m_baseObject->setData(m_offset, data, size);
    }

    SlangResult setObject(IShaderObject* object)
    {
        return m_baseObject->setObject(m_offset, object);
    }

    SlangResult setResource(IResourceView* resourceView)
    {
        return m_baseObject->setResource(m_offset, resourceView);
    }

    SlangResult setSampler(ISamplerState* sampler)
    {
        return m_baseObject->setSampler(m_offset, sampler);
    }

    SlangResult setCombinedTextureSampler(IResourceView* textureView, ISamplerState* sampler)
    {
        return m_baseObject->setCombinedTextureSampler(m_offset, textureView, sampler);
    }
};
}
