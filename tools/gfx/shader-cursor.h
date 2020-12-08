#pragma once

#include "tools/gfx/render.h"
#include "core/slang-basic.h"

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
    ShaderObject* m_baseObject = nullptr;
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

    Result getDereferenced(ShaderCursor& outCursor) const
    {
        switch (m_typeLayout->getKind())
        {
        default:
            return SLANG_E_INVALID_ARG;

        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
            {
                ShaderObject* subObject = m_baseObject->getObject(m_offset);
                outCursor = ShaderCursor(subObject);
                return SLANG_OK;
            }
        }
    }

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
    Result getField(Slang::UnownedStringSlice const& name, ShaderCursor& outCursor)
    {
        // If this cursor is invalid, then can't possible fetch a field.
        //
        if (!isValid())
            return SLANG_E_INVALID_ARG;

        // If the cursor is valid, we want to consider the type of data
        // it is referencing.
        //
        switch (m_typeLayout->getKind())
        {
            // The easy/expected case is when the value has a structure type.
            //
        case slang::TypeReflection::Kind::Struct:
            {
                // We start by looking up the index of a field matching `name`.
                //
                // If there is no such field, we have an error.
                //
                SlangInt fieldIndex = m_typeLayout->findFieldIndexByName(name.begin(), name.end());
                if (fieldIndex == -1)
                    return SLANG_E_INVALID_ARG;

                // Once we know the index of the field being referenced,
                // we create a cursor to point at the field, based on
                // the offset information already in this cursor, plus
                // offsets derived from the field's layout.
                //
                slang::VariableLayoutReflection* fieldLayout =
                    m_typeLayout->getFieldByIndex((unsigned int)fieldIndex);
                ShaderCursor fieldCursor;

                // The field cursorwill point into the same parent object.
                //
                fieldCursor.m_baseObject = m_baseObject;

                // The type being pointed to is the tyep of the field.
                //
                fieldCursor.m_typeLayout = fieldLayout->getTypeLayout();

                // The byte offset is the current offset plus the relative offset of the field.
                // The offset in binding ranges is computed similarly.
                //
                fieldCursor.m_offset.uniformOffset =
                    m_offset.uniformOffset + fieldLayout->getOffset();
                fieldCursor.m_offset.bindingRangeIndex =
                    m_offset.bindingRangeIndex +
                    m_typeLayout->getFieldBindingRangeOffset(fieldIndex);

                // The index of the field within any binding ranges will be the same
                // as the index computed for the parent structure.
                //
                // Note: this case would arise for an array of structures with texture-type
                // fields. Suppose we have:
                //
                //      struct S { Texture2D t; Texture2D u; }
                //      S g[4];
                //
                // In this scenario, `g` holds two binding ranges:
                //
                // * Range #0 comprises 4 textures, representing `g[...].t`
                // * Range #1 comprises 4 textures, representing `g[...].u`
                //
                // A cursor for `g[2]` would have a `bindingRangeIndex` of zero but
                // a `bindingArrayIndex` of 2, iindicating that we could end up
                // referencing either range, but no matter what we know the index
                // is 2. Thus when we form a cursor for `g[2].u` we want to
                // apply the binding range offset to get a `bindingRangeIndex` of
                // 1, while the `bindingArrayIndex` is unmodified.
                //
                // The result is that `g[2].u` is stored in range #1 at array index 2.
                //
                fieldCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex;

                outCursor = fieldCursor;
                return SLANG_OK;
            }
            break;

        // In some cases the user might be trying to acess a field by name
        // from a cursor that references a constant buffer or parameter block,
        // and in these cases we want the access to Just Work.
        //
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
            {
                // We basically need to "dereference" the current cursor
                // to go from a pointer to a constant buffer to a pointer
                // to the *contents* of the constant buffer.
                //
                ShaderCursor d = getDereferenced();
                return d.getField(name, outCursor);
            }
            break;
        }

        return SLANG_E_INVALID_ARG;
    }

    ShaderCursor getField(Slang::UnownedStringSlice const& name)
    {
        ShaderCursor cursor;
        getField(name, cursor);
        return cursor;
    }

    ShaderCursor getField(Slang::String const& name) { return getField(name.getUnownedSlice()); }

    ShaderCursor getElement(Slang::Index index)
    {
        // TODO: need to auto-dereference various buffer types...

        if (m_typeLayout->getKind() == slang::TypeReflection::Kind::Array)
        {
            ShaderCursor elementCursor;
            elementCursor.m_baseObject = m_baseObject;
            elementCursor.m_typeLayout = m_typeLayout->getElementTypeLayout();
            elementCursor.m_offset.uniformOffset =
                m_offset.uniformOffset +
                index * m_typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
            elementCursor.m_offset.bindingRangeIndex = m_offset.bindingRangeIndex;
            elementCursor.m_offset.bindingArrayIndex =
                m_offset.bindingArrayIndex * m_typeLayout->getElementCount() + index;
            return elementCursor;
        }

        return ShaderCursor();
    }

    static int _peek(Slang::UnownedStringSlice const& slice)
    {
        const char* b = slice.begin();
        const char* e = slice.end();
        if (b == e)
            return -1;
        return *b;
    }

    static int _get(Slang::UnownedStringSlice& slice)
    {
        const char* b = slice.begin();
        const char* e = slice.end();
        if (b == e)
            return -1;
        auto result = *b++;
        slice = Slang::UnownedStringSlice(b, e);
        return result;
    }

    static Result followPath(Slang::UnownedStringSlice const& path, ShaderCursor& ioCursor)
    {
        ShaderCursor cursor = ioCursor;

        enum
        {
            ALLOW_NAME = 0x1,
            ALLOW_SUBSCRIPT = 0x2,
            ALLOW_DOT = 0x4,
        };
        int state = ALLOW_NAME | ALLOW_SUBSCRIPT;

        Slang::UnownedStringSlice rest = path;
        for (;;)
        {
            int c = _peek(rest);

            if (c == -1)
                break;
            else if (c == '.')
            {
                if (!(state & ALLOW_DOT))
                    return SLANG_E_INVALID_ARG;

                _get(rest);
                state = ALLOW_NAME;
                continue;
            }
            else if (c == '[')
            {
                if (!(state & ALLOW_SUBSCRIPT))
                    return SLANG_E_INVALID_ARG;

                _get(rest);
                Slang::Index index = 0;
                while (_peek(rest) != ']')
                {
                    int d = _get(rest);
                    if (d >= '0' && d <= '9')
                    {
                        index = index * 10 + (d - '0');
                    }
                    else
                    {
                        return SLANG_E_INVALID_ARG;
                    }
                }

                if (_peek(rest) != ']')
                    return SLANG_E_INVALID_ARG;
                _get(rest);

                cursor = cursor.getElement(index);
                state = ALLOW_DOT | ALLOW_SUBSCRIPT;
                continue;
            }
            else
            {
                const char* nameBegin = rest.begin();
                for (;;)
                {
                    switch (_peek(rest))
                    {
                    default:
                        _get(rest);
                        continue;

                    case -1:
                    case '.':
                    case '[':
                        break;
                    }
                    break;
                }
                char const* nameEnd = rest.begin();
                Slang::UnownedStringSlice name(nameBegin, nameEnd);
                cursor = cursor.getField(name);
                state = ALLOW_DOT | ALLOW_SUBSCRIPT;
                continue;
            }
        }

        ioCursor = cursor;
        return SLANG_OK;
    }

    static Result followPath(Slang::String const& path, ShaderCursor& ioCursor)
    {
        return followPath(path.getUnownedSlice(), ioCursor);
    }

    ShaderCursor getPath(Slang::UnownedStringSlice const& path)
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }

    ShaderCursor getPath(Slang::String const& path)
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }

    ShaderCursor() {}

    ShaderCursor(ShaderObject* object)
        : m_baseObject(object)
        , m_typeLayout(object->getElementTypeLayout())
    {}

    SlangResult setData(void const* data, size_t size)
    {
        return m_baseObject->setData(m_offset, data, size);
    }

    SlangResult setObject(ShaderObject* object)
    {
        return m_baseObject->setObject(m_offset, object);
    }

    SlangResult setResource(ResourceView* resourceView)
    {
        return m_baseObject->setResource(m_offset, resourceView);
    }

    SlangResult setSampler(SamplerState* sampler)
    {
        return m_baseObject->setSampler(m_offset, sampler);
    }

    SlangResult setCombinedTextureSampler(ResourceView* textureView, SamplerState* sampler)
    {
        return m_baseObject->setCombinedTextureSampler(m_offset, textureView, sampler);
    }
};
}
