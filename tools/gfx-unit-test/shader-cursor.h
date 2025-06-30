#pragma once

#include "slang-rhi.h"

namespace rhi {

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
    ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;
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

    ShaderCursor getDereferenced() const
    {
        ShaderCursor result;
        getDereferenced(result);
        return result;
    }

    /// Form a cursor pointing to the field with the given `name` within the value this cursor
    /// points at.
    ///
    /// If the operation succeeds, then the field cursor is written to `outCursor`.
    Result getField(const char* nameBegin, const char* nameEnd, ShaderCursor& outCursor) const;

    ShaderCursor getField(const char* name) const
    {
        ShaderCursor cursor;
        getField(name, nullptr, cursor);
        return cursor;
    }

    /// Some resources such as RWStructuredBuffer, AppendStructuredBuffer and
    /// ConsumeStructuredBuffer need to have their counter explicitly bound on
    /// APIs other than DirectX, this will return a valid ShaderCursor pointing
    /// to that resource if that is the case.
    /// Otherwise, this returns an invalid cursor.
    ShaderCursor getExplicitCounter() const;

    ShaderCursor getElement(uint32_t index) const;

    static Result followPath(const char* path, ShaderCursor& ioCursor);

    ShaderCursor getPath(const char* path) const
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }

    ShaderCursor() {}

    ShaderCursor(IShaderObject* object)
        : m_baseObject(object)
        , m_typeLayout(object->getElementTypeLayout())
        , m_containerType(object->getContainerType())
    {
    }

    Result setData(const void* data, Size size) const { return m_baseObject->setData(m_offset, data, size); }

    template<typename T>
    Result setData(const T& data) const
    {
        return setData(&data, sizeof(data));
    }

    Result setObject(IShaderObject* object) const { return m_baseObject->setObject(m_offset, object); }

    Result setBinding(const Binding& binding) const { return m_baseObject->setBinding(m_offset, binding); }

    Result setDescriptorHandle(const DescriptorHandle& handle) const
    {
        return m_baseObject->setDescriptorHandle(m_offset, handle);
    }

    Result setSpecializationArgs(const slang::SpecializationArg* args, uint32_t count) const
    {
        return m_baseObject->setSpecializationArgs(m_offset, args, count);
    }

    /// Produce a cursor to the field with the given `name`.
    ///
    /// This is a convenience wrapper around `getField()`.
    ShaderCursor operator[](const char* name) const { return getField(name); }

    /// Produce a cursor to the element or field with the given `index`.
    ///
    /// This is a convenience wrapper around `getElement()`.
    ShaderCursor operator[](int64_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](uint64_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](int32_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](uint32_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](int16_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](uint16_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](int8_t index) const { return getElement((uint32_t)index); }
    ShaderCursor operator[](uint8_t index) const { return getElement((uint32_t)index); }
};

inline Result ShaderCursor::getDereferenced(ShaderCursor& outCursor) const
{
    switch (m_typeLayout->getKind())
    {
    default:
        return SLANG_E_INVALID_ARG;

    case slang::TypeReflection::Kind::ConstantBuffer:
    case slang::TypeReflection::Kind::ParameterBlock:
    {
        auto subObject = m_baseObject->getObject(m_offset);
        outCursor = ShaderCursor(subObject);
        return SLANG_OK;
    }
    }
}

inline Result ShaderCursor::getField(const char* name, const char* nameEnd, ShaderCursor& outCursor) const
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
        SlangInt fieldIndex = m_typeLayout->findFieldIndexByName(name, nameEnd);
        if (fieldIndex == -1)
            break;

        // Once we know the index of the field being referenced,
        // we create a cursor to point at the field, based on
        // the offset information already in this cursor, plus
        // offsets derived from the field's layout.
        //
        slang::VariableLayoutReflection* fieldLayout = m_typeLayout->getFieldByIndex((unsigned int)fieldIndex);
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
        fieldCursor.m_offset.uniformOffset = m_offset.uniformOffset + fieldLayout->getOffset();
        fieldCursor.m_offset.bindingRangeIndex =
            m_offset.bindingRangeIndex + (uint32_t)m_typeLayout->getFieldBindingRangeOffset(fieldIndex);

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
        break;
    }
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
        return d.getField(name, nameEnd, outCursor);
        break;
    }
    default:
        break;
    }

    // If a cursor is pointing at a root shader object (created for a
    // program), then we will also iterate over the entry point shader
    // objects attached to it and look for a matching parameter name
    // on them.
    //
    // This is a bit of "do what I mean" logic and could potentially
    // lead to problems if there could be multiple entry points with
    // the same parameter name.
    //
    // TODO: figure out whether we should support this long-term.
    //
    uint32_t entryPointCount = m_baseObject->getEntryPointCount();
    for (uint32_t e = 0; e < entryPointCount; ++e)
    {
        ComPtr<IShaderObject> entryPoint;
        m_baseObject->getEntryPoint(e, entryPoint.writeRef());

        ShaderCursor entryPointCursor(entryPoint);

        auto result = entryPointCursor.getField(name, nameEnd, outCursor);
        if (SLANG_SUCCEEDED(result))
            return result;
    }

    return SLANG_E_INVALID_ARG;
}

inline ShaderCursor ShaderCursor::getExplicitCounter() const
{
    // Similar to getField below

    // The alternative to handling this here would be to augment IResourceView
    // with a `getCounterResourceView()`, and set that also in `setResource`
    if (const auto counterVarLayout = m_typeLayout->getExplicitCounter())
    {
        ShaderCursor counterCursor;

        // The counter cursor will point into the same parent object.
        counterCursor.m_baseObject = m_baseObject;

        // The type being pointed to is the type of the field.
        counterCursor.m_typeLayout = counterVarLayout->getTypeLayout();

        // The byte offset is the current offset plus the relative offset of the counter.
        // The offset in binding ranges is computed similarly.
        counterCursor.m_offset.uniformOffset = m_offset.uniformOffset + SlangInt(counterVarLayout->getOffset());
        counterCursor.m_offset.bindingRangeIndex =
            m_offset.bindingRangeIndex + uint32_t(m_typeLayout->getExplicitCounterBindingRangeOffset());

        // The index of the counter within any binding ranges will be the same
        // as the index computed for the parent structure.
        //
        // Note: this case would arise for an array of structured buffers
        //
        //      AppendStructuredBuffer g[4];
        //
        // In this scenario, `g` holds two binding ranges:
        //
        // * Range #0 comprises 4 element buffers, representing `g[...].elements`
        // * Range #1 comprises 4 counter buffers, representing `g[...].counter`
        //
        // A cursor for `g[2]` would have a `bindingRangeIndex` of zero but
        // a `bindingArrayIndex` of 2, indicating that we could end up
        // referencing either range, but no matter what we know the index
        // is 2. Thus when we form a cursor for `g[2].counter` we want to
        // apply the binding range offset to get a `bindingRangeIndex` of
        // 1, while the `bindingArrayIndex` is unmodified.
        //
        // The result is that `g[2].counter` is stored in range #1 at array index 2.
        //
        counterCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex;

        return counterCursor;
    }
    // Otherwise, return an invalid cursor
    return ShaderCursor{};
}

inline ShaderCursor ShaderCursor::getElement(uint32_t index) const
{
    if (m_containerType != ShaderObjectContainerType::None)
    {
        ShaderCursor elementCursor;
        elementCursor.m_baseObject = m_baseObject;
        elementCursor.m_typeLayout = m_typeLayout->getElementTypeLayout();
        elementCursor.m_containerType = m_containerType;
        elementCursor.m_offset.uniformOffset = index * m_typeLayout->getStride();
        elementCursor.m_offset.bindingRangeIndex = 0;
        elementCursor.m_offset.bindingArrayIndex = index;
        return elementCursor;
    }

    switch (m_typeLayout->getKind())
    {
    case slang::TypeReflection::Kind::Array:
    {
        ShaderCursor elementCursor;
        elementCursor.m_baseObject = m_baseObject;
        elementCursor.m_typeLayout = m_typeLayout->getElementTypeLayout();
        elementCursor.m_offset.uniformOffset =
            m_offset.uniformOffset + index * m_typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
        elementCursor.m_offset.bindingRangeIndex = m_offset.bindingRangeIndex;
        elementCursor.m_offset.bindingArrayIndex =
            m_offset.bindingArrayIndex * (uint32_t)m_typeLayout->getElementCount() + index;
        return elementCursor;
        break;
    }
    case slang::TypeReflection::Kind::Struct:
    {
        // The logic here is similar to `getField()` except that we don't
        // need to look up the field index based on a name first.
        //
        auto fieldIndex = index;
        slang::VariableLayoutReflection* fieldLayout = m_typeLayout->getFieldByIndex((unsigned int)fieldIndex);
        if (!fieldLayout)
            return ShaderCursor();

        ShaderCursor fieldCursor;
        fieldCursor.m_baseObject = m_baseObject;
        fieldCursor.m_typeLayout = fieldLayout->getTypeLayout();
        fieldCursor.m_offset.uniformOffset = m_offset.uniformOffset + fieldLayout->getOffset();
        fieldCursor.m_offset.bindingRangeIndex =
            m_offset.bindingRangeIndex + (uint32_t)m_typeLayout->getFieldBindingRangeOffset(fieldIndex);
        fieldCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex;
        return fieldCursor;
        break;
    }
    case slang::TypeReflection::Kind::Vector:
    case slang::TypeReflection::Kind::Matrix:
    {
        ShaderCursor fieldCursor;
        fieldCursor.m_baseObject = m_baseObject;
        fieldCursor.m_typeLayout = m_typeLayout->getElementTypeLayout();
        fieldCursor.m_offset.uniformOffset =
            m_offset.uniformOffset + m_typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM) * index;
        fieldCursor.m_offset.bindingRangeIndex = m_offset.bindingRangeIndex;
        fieldCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex;
        return fieldCursor;
        break;
    }
    default:
        break;
    }

    return ShaderCursor();
}

namespace detail {

inline int peek(const char* slice)
{
    const char* b = slice;
    if (!b || !*b)
        return -1;
    return *b;
}

inline int get(const char*& slice)
{
    const char* b = slice;
    if (!b || !*b)
        return -1;
    auto result = *b++;
    slice = b;
    return result;
}

} // namespace detail

inline Result ShaderCursor::followPath(const char* path, ShaderCursor& ioCursor)
{
    ShaderCursor cursor = ioCursor;

    enum
    {
        ALLOW_NAME = 0x1,
        ALLOW_SUBSCRIPT = 0x2,
        ALLOW_DOT = 0x4,
    };
    int state = ALLOW_NAME | ALLOW_SUBSCRIPT;

    const char* rest = path;
    for (;;)
    {
        int c = detail::peek(rest);

        if (c == -1)
            break;
        else if (c == '.')
        {
            if (!(state & ALLOW_DOT))
                return SLANG_E_INVALID_ARG;

            detail::get(rest);
            state = ALLOW_NAME;
            continue;
        }
        else if (c == '[')
        {
            if (!(state & ALLOW_SUBSCRIPT))
                return SLANG_E_INVALID_ARG;

            detail::get(rest);
            uint32_t index = 0;
            while (detail::peek(rest) != ']')
            {
                int d = detail::get(rest);
                if (d >= '0' && d <= '9')
                {
                    index = index * 10 + (d - '0');
                }
                else
                {
                    return SLANG_E_INVALID_ARG;
                }
            }

            if (detail::peek(rest) != ']')
                return SLANG_E_INVALID_ARG;
            detail::get(rest);

            cursor = cursor.getElement(index);
            state = ALLOW_DOT | ALLOW_SUBSCRIPT;
            continue;
        }
        else
        {
            const char* nameBegin = rest;
            for (;;)
            {
                switch (detail::peek(rest))
                {
                default:
                    detail::get(rest);
                    continue;

                case -1:
                case '.':
                case '[':
                    break;
                }
                break;
            }
            const char* nameEnd = rest;
            ShaderCursor newCursor;
            cursor.getField(nameBegin, nameEnd, newCursor);
            cursor = newCursor;
            state = ALLOW_DOT | ALLOW_SUBSCRIPT;
            continue;
        }
    }

    ioCursor = cursor;
    return SLANG_OK;
}

} // namespace rhi
