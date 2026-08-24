#ifndef SLANG_REFLECTION_CURSOR_H
#define SLANG_REFLECTION_CURSOR_H

// slang-reflection-cursor.h
//
// EXPERIMENTAL: NOT part of the stable Slang API contract. The types, names, and behavior here may
// change or be removed in any release without notice, and this makes no ABI-stability guarantee. Do
// not depend on it from production code. See issue shader-slang/slang#12183.
//
// A header-only utility for computing the cumulative offset of a parameter reached by an explicit
// path through reflection layouts. The shipping reflection API reports only per-link offsets
// (`VariableLayoutReflection::getOffset`/`getBindingSpace` give a field's placement within its
// immediate parent). Summing those along a path, and applying the container rules for where byte
// offsets and register spaces reset, is what `examples/reflection-api/main.cpp`
// (`calculateCumulativeOffset`) demonstrates; this lifts that logic into a reusable form.
//
// A cumulative offset is always relative to *some* root: the program layout, or the buffer/block
// most recently entered on the path. A nested field's byte offset, for instance, is cumulative but
// still relative to the start of its enclosing constant buffer.

#include "slang.h"

#include <cassert>
#include <exception>
#include <vector>

namespace slang
{
namespace experimental
{
namespace reflection
{

/// Thrown by a `Cursor` navigation operation that is invalid for the type the cursor currently
/// points at (for example, navigating to a field when the current type is not a struct). The cursor
/// is left unchanged, so it remains usable after the exception is caught.
///
/// It is the client's responsibility to only invoke operations compatible with the current type —
/// by reflecting on that type first, or by a convention maintained in their own code. Navigation is
/// not an existence test; catching this to probe for a field or element is a misuse.
class NavigationError : public std::exception
{
public:
    explicit NavigationError(const char* message)
        : m_message(message)
    {
    }
    const char* what() const noexcept override { return m_message; }

private:
    const char* m_message;
};

/// A cumulative offset resolved in one layout unit. `offset` is the value in that unit (e.g. bytes
/// for `Uniform`, a binding/register index for resource units); `space` is the associated register
/// space or descriptor set, meaningful only for the space-carrying resource units. Both are
/// relative to the offset's root (the program layout, or the most recently entered buffer/block).
struct CumulativeOffset
{
    size_t offset = 0;
    size_t space = 0;
};

/// One step along an `AccessPath`: either a variable (a struct field, a scope, or the content of an
/// entered buffer/block) or an element of an array or structured buffer chosen by index. A link
/// contributes its own offset to the cumulative sum, and may mark a container boundary where the
/// byte-offset root resets — a constant buffer or structured buffer always, a parameter block
/// (which also resets resource spaces) additionally.
class AccessPathLink
{
public:
    /// A link for a variable layout (field, scope, or entered-container content). Contributes the
    /// variable's own `getOffset`/`getBindingSpace`.
    static AccessPathLink variable(VariableLayoutReflection* variableLayout)
    {
        return AccessPathLink(variableLayout, nullptr, 0, StrideKind::ArrayElement);
    }

    /// A link for element `elementIndex` of an array whose type layout is `arrayTypeLayout`.
    /// Contributes `elementIndex` times the array's per-element stride.
    static AccessPathLink arrayElement(TypeLayoutReflection* arrayTypeLayout, size_t elementIndex)
    {
        return AccessPathLink(nullptr, arrayTypeLayout, elementIndex, StrideKind::ArrayElement);
    }

    /// A link for element `elementIndex` of a structured buffer whose element type layout is
    /// `elementTypeLayout`. Contributes `elementIndex` times that element type's stride, which is
    /// how a structured buffer spaces its elements (reflection reports no `getElementStride` for
    /// the buffer type itself).
    static AccessPathLink bufferElement(
        TypeLayoutReflection* elementTypeLayout,
        size_t elementIndex)
    {
        return AccessPathLink(nullptr, elementTypeLayout, elementIndex, StrideKind::TypeStride);
    }

    bool isConstantBufferBoundary() const { return m_isConstantBufferBoundary; }
    bool isParameterBlockBoundary() const { return m_isParameterBlockBoundary; }

    /// Record that the container whose content follows this link resets the byte-offset root: a
    /// constant buffer or structured buffer always, a parameter block (which also introduces its
    /// own sub-element register space) additionally. The stored flag is named for the
    /// constant-buffer case but applies equally to any container that resets the Uniform root.
    void markAsContainerBoundary(bool isParameterBlock)
    {
        m_isConstantBufferBoundary = true;
        if (isParameterBlock)
            m_isParameterBlockBoundary = true;
    }

private:
    // Only AccessPath accumulates links, via the per-link contributions below; they stay private so
    // no per-link offset method is exposed to clients (the public offset queries are
    // AccessPath::calcCumulativeOffset and Cursor's forwarding wrapper).
    friend class AccessPath;

    // How an indexed link derives its per-element stride: from the array type's own element stride,
    // or from the element type's stride (structured buffers, which expose no element-stride
    // accessor on the buffer type itself).
    enum class StrideKind
    {
        ArrayElement,
        TypeStride,
    };

    // Represented as a tagged union with the invariant: exactly one of m_variableLayout /
    // m_indexedTypeLayout is non-null. The private constructor and the factories are the only ways
    // to build a link, so the invariant holds by construction; assertClassified() re-checks it at
    // each point of use.
    AccessPathLink(
        VariableLayoutReflection* variableLayout,
        TypeLayoutReflection* indexedTypeLayout,
        size_t elementIndex,
        StrideKind strideKind)
        : m_variableLayout(variableLayout)
        , m_indexedTypeLayout(indexedTypeLayout)
        , m_elementIndex(elementIndex)
        , m_strideKind(strideKind)
    {
        assertClassified();
    }

    void assertClassified() const
    {
        assert((m_variableLayout != nullptr) != (m_indexedTypeLayout != nullptr));
    }

    // This link's own contribution to the offset value in `unit`.
    size_t calcOffset(ParameterCategory unit) const
    {
        assertClassified();
        if (m_variableLayout)
            return m_variableLayout->getOffset(unit);
        SlangParameterCategory cat = (SlangParameterCategory)unit;
        size_t stride = m_strideKind == StrideKind::ArrayElement
                            ? m_indexedTypeLayout->getElementStride(cat)
                            : m_indexedTypeLayout->getStride(cat);
        return m_elementIndex * stride;
    }

    // This link's own contribution to the space/set in `unit` (indexed elements contribute none).
    size_t calcSpace(ParameterCategory unit) const
    {
        assertClassified();
        return m_variableLayout ? m_variableLayout->getBindingSpace(unit) : 0;
    }

    VariableLayoutReflection* m_variableLayout;
    TypeLayoutReflection* m_indexedTypeLayout;
    size_t m_elementIndex;
    StrideKind m_strideKind;
    bool m_isConstantBufferBoundary = false;
    bool m_isParameterBlockBoundary = false;
};

/// An explicit path from a root to a designated location in a program's reflection layout, from
/// which a cumulative offset can be computed. A path is required (rather than a
/// `(target, ancestor)` pair) because the same nested type can occur at more than one location: for
/// `ConstantBuffer<Outer> gOuter` with `struct Outer { Inner a; Inner b; }`, `gOuter.a.x` and
/// `gOuter.b.x` are distinct offsets that only a path can name.
///
/// This type does not own or retain the reflection objects that contribute to it. The client must
/// ensure the `ProgramLayout` (and the component/session that produced it) which owns those objects
/// outlives any `AccessPath`, and any `Cursor` built from it.
class AccessPath
{
public:
    bool isEmpty() const { return m_links.empty(); }
    size_t getLinkCount() const { return m_links.size(); }

    /// Append a link built by `AccessPathLink::variable` / `::arrayElement` / `::bufferElement`.
    void add(const AccessPathLink& link) { m_links.push_back(link); }

    /// Record a container boundary on the current leaf (the container variable, whose content will
    /// be appended next). Does nothing on an empty path — a path rooted directly at a container
    /// type has no outer link whose offset would need to be excluded.
    void markLeafAsContainerBoundary(bool isParameterBlock)
    {
        if (!m_links.empty())
            m_links.back().markAsContainerBoundary(isParameterBlock);
    }

    /// Compute the cumulative offset of the leaf in `unit`. This is a computation over every link,
    /// not a stored value; invoke it once and reuse the result rather than calling it repeatedly.
    CumulativeOffset calcCumulativeOffset(ParameterCategory unit) const
    {
        // Ports the three cases of `calculateCumulativeOffset` from the reflection example, with
        // the path stored leaf-last (index 0 = root). Byte offsets do not leak outside their
        // constant buffer, so `Uniform` stops at the deepest constant-buffer boundary. Resource
        // units sum offset+space up to the deepest parameter-block boundary, then — because a
        // parameter block occupies its own register space — add each outer link's sub-element
        // register space.
        CumulativeOffset result;

        const int count = (int)m_links.size();
        const int deepestConstantBuffer = deepestBoundaryIndex(/*parameterBlockOnly*/ false);
        const int deepestParameterBlock = deepestBoundaryIndex(/*parameterBlockOnly*/ true);

        switch (unit)
        {
        default:
            for (int i = count - 1; i >= 0; --i)
                result.offset += m_links[i].calcOffset(unit);
            break;

        case ParameterCategory::Uniform:
            for (int i = count - 1; i > deepestConstantBuffer; --i)
                result.offset += m_links[i].calcOffset(unit);
            break;

        case ParameterCategory::ConstantBuffer:
        case ParameterCategory::ShaderResource:
        case ParameterCategory::UnorderedAccess:
        case ParameterCategory::SamplerState:
        case ParameterCategory::DescriptorTableSlot:
            for (int i = count - 1; i > deepestParameterBlock; --i)
            {
                result.offset += m_links[i].calcOffset(unit);
                result.space += m_links[i].calcSpace(unit);
            }
            for (int i = deepestParameterBlock; i >= 0; --i)
                result.space += m_links[i].calcOffset(ParameterCategory::SubElementRegisterSpace);
            break;
        }

        return result;
    }

private:
    // Index of the boundary link closest to the leaf, or -1 if none. A -1 makes the first loop of
    // each case above cover every link and the second loop run zero times, matching the reference
    // algorithm's null-marker behavior.
    int deepestBoundaryIndex(bool parameterBlockOnly) const
    {
        for (int i = (int)m_links.size() - 1; i >= 0; --i)
        {
            const AccessPathLink& link = m_links[i];
            if (parameterBlockOnly ? link.isParameterBlockBoundary()
                                   : link.isConstantBufferBoundary())
                return i;
        }
        return -1;
    }

    std::vector<AccessPathLink> m_links;
};

/// A cursor that builds an `AccessPath` by navigating a program's reflection layout and computes
/// the cumulative offset at its current location. Construct it at a program's global scope or at a
/// bare type layout, then step to struct fields, array elements, entry points, or into a
/// buffer/block.
///
/// Navigation is type-centric: each operation is valid only for the kind the cursor currently
/// points at, and throws `NavigationError` (leaving the cursor unchanged) otherwise. It is the
/// client's job to only invoke compatible operations; navigation is not a way to test whether a
/// field or element exists.
///
/// Typical use:
///
///     slang::experimental::reflection::Cursor cursor(programLayout);
///     cursor.navigateToFieldByName("gOuter"); // the ConstantBuffer<Outer> field
///     cursor.navigateToContent();             // step inside the constant buffer
///     cursor.navigateToFieldByName("b");       // Outer::b
///     cursor.navigateToFieldByName("x");       // Inner::x
///     auto off = cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset;
///
/// Like `AccessPath`, a cursor does not retain the reflection objects it points at; the owning
/// `ProgramLayout` must outlive the cursor.
class Cursor
{
public:
    /// Root the cursor at a program's global-scope parameters.
    explicit Cursor(ProgramLayout* programLayout)
        : m_programLayout(programLayout)
    {
        if (programLayout)
        {
            if (VariableLayoutReflection* globals = programLayout->getGlobalParamsVarLayout())
            {
                m_path.add(AccessPathLink::variable(globals));
                m_typeLayout = globals->getTypeLayout();
            }
        }
    }

    /// Root the cursor at a bare type layout; offsets are relative to that type's origin.
    explicit Cursor(TypeLayoutReflection* typeLayout)
        : m_typeLayout(typeLayout)
    {
    }

    /// The type layout the cursor currently points at, or null if it was rooted at a null layout.
    TypeLayoutReflection* getTypeLayout() const { return m_typeLayout; }

    const AccessPath& getAccessPath() const { return m_path; }

    /// Compute the cumulative offset at the current location in `unit`. This is a computation over
    /// the whole path; invoke it once and reuse the result.
    CumulativeOffset calcCumulativeOffset(ParameterCategory unit) const
    {
        return m_path.calcCumulativeOffset(unit);
    }

    /// Re-root the cursor at the given entry point's parameter scope, discarding any prior path.
    /// Requires a cursor that was constructed from a `ProgramLayout`.
    void navigateToEntryPointByIndex(SlangUInt index)
    {
        if (!m_programLayout)
            throw NavigationError("navigateToEntryPoint requires a program-rooted cursor");
        if (index >= m_programLayout->getEntryPointCount())
            throw NavigationError("entry point index out of range");
        rerootAtEntryPoint(m_programLayout->getEntryPointByIndex(index));
    }

    /// Re-root the cursor at the named entry point's parameter scope, discarding any prior path.
    /// Requires a cursor that was constructed from a `ProgramLayout`.
    void navigateToEntryPointByName(const char* name)
    {
        if (!m_programLayout)
            throw NavigationError("navigateToEntryPoint requires a program-rooted cursor");
        if (!name)
            throw NavigationError("entry point name is null");
        EntryPointReflection* entryPoint = m_programLayout->findEntryPointByName(name);
        if (!entryPoint)
            throw NavigationError("no entry point with the given name");
        rerootAtEntryPoint(entryPoint);
    }

    /// Navigate to a struct field by name. Valid only when the current type is a struct.
    void navigateToFieldByName(const char* name)
    {
        requireStruct();
        if (!name)
            throw NavigationError("field name is null");
        SlangInt index = m_typeLayout->findFieldIndexByName(name);
        if (index < 0)
            throw NavigationError("no field with the given name");
        appendField((unsigned int)index);
    }

    /// Navigate to a struct field by index. Valid only when the current type is a struct.
    void navigateToFieldByIndex(unsigned int index)
    {
        requireStruct();
        if (index >= m_typeLayout->getFieldCount())
            throw NavigationError("field index out of range");
        appendField(index);
    }

    /// Navigate to an array element by index. Valid only when the current type is an array. Adds
    /// `index * getElementStride(unit)`; an array element does not reset the offset root.
    void navigateToElement(size_t index)
    {
        if (!m_typeLayout || m_typeLayout->getKind() != TypeReflection::Kind::Array)
            throw NavigationError("navigateToElement requires an array type");
        size_t count = m_typeLayout->getElementCount();
        if (count != SLANG_UNBOUNDED_SIZE && count != SLANG_UNKNOWN_SIZE && index >= count)
            throw NavigationError("array element index out of range");
        m_path.add(AccessPathLink::arrayElement(m_typeLayout, index));
        m_typeLayout = m_typeLayout->getElementTypeLayout();
    }

    /// Step inside a uniform parameter group — `ConstantBuffer<>`, `ParameterBlock<>`, or
    /// `TextureBuffer<>` — to point at its content.
    ///
    /// After this step the offset root may change: a constant buffer resets the origin of byte
    /// (`Uniform`) offsets, and a parameter block additionally resets resource spaces, so a
    /// cumulative offset queried past this point is relative to this buffer/block rather than the
    /// outer one. Kinds that are not uniform parameter groups (e.g. a shader storage buffer) have
    /// no content in this sense and throw `NavigationError`.
    void navigateToContent()
    {
        if (!m_typeLayout)
            throw NavigationError("navigateToContent requires a uniform parameter group");
        switch (m_typeLayout->getKind())
        {
        case TypeReflection::Kind::ConstantBuffer:
        case TypeReflection::Kind::ParameterBlock:
        case TypeReflection::Kind::TextureBuffer:
            break;
        default:
            throw NavigationError("navigateToContent requires a uniform parameter group");
        }

        // The three admitted kinds are the uniform parameter groups, whose layout always exposes
        // both a content element and a container var layout; the asserts guard against a future
        // kind being added to the switch above that does not.
        VariableLayoutReflection* element = m_typeLayout->getElementVarLayout();
        assert(element != nullptr);
        VariableLayoutReflection* container = m_typeLayout->getContainerVarLayout();
        assert(container != nullptr);

        // A parameter block occupies its own register space, which reflection reports as a non-zero
        // SubElementRegisterSpace size on the container; a plain constant buffer does not.
        bool isParameterBlock =
            container->getTypeLayout()->getSize(ParameterCategory::SubElementRegisterSpace) != 0;

        // Mark the container's link (the current leaf) before appending the content link, so the
        // boundary sits on the outer variable, matching the reference algorithm.
        m_path.markLeafAsContainerBoundary(isParameterBlock);

        m_path.add(AccessPathLink::variable(element));
        m_typeLayout = element->getTypeLayout();
    }

    /// Navigate to element `index` of a structured buffer (`StructuredBuffer<>` /
    /// `RWStructuredBuffer<>`, reflected as a resource of structured-buffer shape), pointing at the
    /// element type.
    ///
    /// This is a single operation rather than a `navigateToContent` followed by `navigateToElement`
    /// because reflection does not expose a structured buffer's content as an array type layout
    /// with its own element stride (see the note below), so there is no intermediate array cursor
    /// to stop at. Like entering a constant buffer, it resets the origin of byte (`Uniform`)
    /// offsets: the element's fields are addressed relative to the buffer's data origin
    /// (`index * elementStride` plus any field offset within the element), independent of any
    /// uniform offset the buffer itself carries within an enclosing struct or buffer. Navigating
    /// directly to a field or plain array element of a bare structured buffer is not allowed (for
    /// the same reason it is not allowed on a constant buffer); use this instead.
    ///
    /// A GLSL shader-storage buffer (`Kind::ShaderStorageBuffer`) is not supported: the current
    /// reflection API exposes no element type layout for it, so there is no element to step onto
    /// (see shader-slang/slang#12776). Calling this on one throws `NavigationError`.
    void navigateToStructuredBufferElement(size_t index)
    {
        if (!m_typeLayout || !isStructuredBuffer(m_typeLayout))
            throw NavigationError("navigateToStructuredBufferElement requires a structured buffer");

        // The element type layout carries the element's field offsets and its own stride;
        // reflection reports no getElementStride() on the buffer type itself, so the stride comes
        // from the element type. A structured buffer's contents live in their own storage, so an
        // element offset must be relative to the buffer's data origin, not to any enclosing uniform
        // block: when the buffer sits inside a struct that follows other uniform data (e.g.
        // `struct S { float4 prefix; RWStructuredBuffer<T> sb; }`), that ancestor's uniform offset
        // must not leak in. So mark the buffer's own link (the current leaf) as a
        // constant-buffer-style boundary, stopping the Uniform accumulation here (not a parameter
        // block: a structured buffer does not introduce its own sub-element register space).
        TypeLayoutReflection* elementType = m_typeLayout->getElementTypeLayout();
        assert(elementType != nullptr); // guaranteed for the structured-buffer resource kind above
        m_path.markLeafAsContainerBoundary(/*isParameterBlock*/ false);

        m_path.add(AccessPathLink::bufferElement(elementType, index));
        m_typeLayout = elementType;
    }

private:
    // An HLSL StructuredBuffer/RWStructuredBuffer reflects as a resource of structured-buffer shape
    // and exposes its element type via getElementTypeLayout(). A GLSL shader-storage buffer
    // (Kind::ShaderStorageBuffer) is intentionally excluded: the current reflection API exposes no
    // element type layout for it, so navigateToStructuredBufferElement cannot step into it. See the
    // note on that method and shader-slang/slang#12776.
    static bool isStructuredBuffer(TypeLayoutReflection* typeLayout)
    {
        return typeLayout->getKind() == TypeReflection::Kind::Resource &&
               (typeLayout->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) ==
                   SLANG_STRUCTURED_BUFFER;
    }

    void requireStruct()
    {
        if (!m_typeLayout || m_typeLayout->getKind() != TypeReflection::Kind::Struct)
            throw NavigationError("navigateToField requires a struct type");
    }

    void appendField(unsigned int index)
    {
        VariableLayoutReflection* field = m_typeLayout->getFieldByIndex(index);
        m_path.add(AccessPathLink::variable(field));
        m_typeLayout = field->getTypeLayout();
    }

    void rerootAtEntryPoint(EntryPointReflection* entryPoint)
    {
        VariableLayoutReflection* scope = entryPoint->getVarLayout();
        m_path = AccessPath();
        m_path.add(AccessPathLink::variable(scope));
        m_typeLayout = scope->getTypeLayout();
    }

    ProgramLayout* m_programLayout = nullptr;
    AccessPath m_path;
    TypeLayoutReflection* m_typeLayout = nullptr;
};

} // namespace reflection
} // namespace experimental
} // namespace slang

#endif // SLANG_REFLECTION_CURSOR_H
