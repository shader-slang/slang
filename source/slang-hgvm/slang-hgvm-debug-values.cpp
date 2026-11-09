#include "core/slang-math.h"
#include "slang-hgvm-debug-session.h"

#include <charconv>
#include <cstdio>
#include <cstring>

namespace Slang::HGVM
{
bool DebugSession::getVariableContext(Index frame, VariableContext& context) const
{
    if (!m_stopped || frame < 0)
        return false;
    for (Index i = m_callStack.getCount(); i-- > 0;)
    {
        const auto& call = m_callStack[i];
        const auto& scope = m_map->getScope(call.scopeIndex);
        context.call = call.interpreterCall;
        context.scope = scope.scope;
        context.inlinedAt = scope.inlinedAt;
        context.pc =
            i + 1 == m_callStack.getCount() ? m_address.wordOffset : m_callStack[i + 1].callSite;
        for (const auto& function : m_map->getFunctions())
            if (function.entry <= context.pc && context.pc < function.end)
                context.function = function.id;
        for (;;)
        {
            if (frame-- == 0)
                return true;
            const auto* site = m_map->findInlineSite(context.inlinedAt);
            if (!site)
                break;
            context.scope = site->scope;
            context.inlinedAt = site->parent;
        }
    }
    return false;
}

List<const DebugVariableInfo*> DebugSession::getVisibleVariables(
    const VariableContext& context) const
{
    List<const DebugVariableInfo*> result;
    HashSet<HGBCId> visited;
    for (HGBCId scope = context.scope; scope && visited.add(scope);
         scope = m_map->getParentScope(scope))
        for (const auto& variable : m_map->getVariables())
            if (variable.function == context.function && variable.scope == scope &&
                variable.inlinedAt == context.inlinedAt && variable.declaration <= context.pc)
                result.add(&variable);
    return result;
}

/// Decodes values on demand using interpreter-validated reads and canonical staged layouts.
struct DebugSession::ValueReader
{
public:
    Interpreter* interpreter;
    DebugMap* map;
    DebugCallId call;
    String error;
    DebugSession* session = nullptr;
    bool lazyChildren = false;
    bool allowBufferReadback = false;
    bool typeOnly = false;

    bool fail(const char* message)
    {
        error = message;
        return false;
    }

    String typeName(HGBCId type) const
    {
        if (auto metadata = map->findType(type))
        {
            if (metadata->qualifiedName.getLength())
                // HGBC reflection names omit modules but retain namespaces and generic arguments.
                return metadata->qualifiedName;
            if (auto layout = map->findTypeLayout(type))
                if (layout->kind == TypeKind::Address)
                {
                    const char* prefix = metadata->opcode == HGBCOpcode::RefType     ? "__ref "
                                         : metadata->opcode == HGBCOpcode::OutType   ? "out "
                                         : metadata->opcode == HGBCOpcode::InOutType ? "inout "
                                         : metadata->opcode == HGBCOpcode::BorrowInType
                                             ? "borrowin "
                                             : "__ptr ";
                    return String(prefix) + typeName(layout->valueType);
                }
            return getHGBCOpcodeInfo(metadata->opcode)->name;
        }
        return "<unknown type>";
    }

    String typeName(const ValueLocation& location) const
    {
        String name = typeName(location.type);
        return location.rowWidth ? String("vector<") + name + ", " + String(location.rowWidth) + ">"
                                 : name;
    }

    bool read(const ValueLocation& value, void* bytes, uint32_t size)
    {
        if (value.snapshot >= 0)
        {
            const auto& data = session->m_bufferSnapshots[value.snapshot].bytes;
            if (value.offset > uint64_t(data.getCount()) ||
                size > uint64_t(data.getCount()) - value.offset)
                return fail("Cannot read outside the buffer snapshot.");
            if (size)
                std::memcpy(bytes, data.getBuffer() + value.offset, size);
            return true;
        }
        return interpreter->readDebugMemory(call, value.address, value.offset, bytes, size) ||
               fail("Cannot read this address.");
    }

    /// Reads the live buffer description and checks it against the variable's element type.
    bool bufferInfo(const ValueLocation& value, Address& handle, DebugBufferInfo& info)
    {
        if (!read(value, &handle, sizeof(handle)))
            return false;
        if (!interpreter->getDebugBufferInfo(call, handle, info))
            return fail("Cannot read this buffer (null or no longer live).");
        if (info.elementType != map->findTypeLayout(value.type)->elementType)
            return fail("Buffer element type does not match the inspected value.");
        return true;
    }

    /// Reads only the descriptor; mip enumeration never needs GPU synchronization.
    bool textureInfo(const ValueLocation& value, Address& handle, TextureInfo& info)
    {
        if (!read(value, &handle, sizeof(handle)))
            return false;
        return interpreter->getDebugTextureInfo(call, handle, info) ||
               fail("Cannot read this texture (null or no longer live).");
    }

    /// Caches a requested range once per stop, shared by aliases and nested field inspection.
    /// A child stores a snapshot index, so adding another range never invalidates its storage.
    Index bufferSnapshot(const ValueLocation& value, uint32_t start, uint32_t count)
    {
        Address handle;
        DebugBufferInfo info;
        if (!bufferInfo(value, handle, info))
            return -1;
        if (start > info.elementCount || count > info.elementCount - start)
        {
            fail("Buffer index out of bounds.");
            return -1;
        }
        for (Index i = 0; i < session->m_bufferSnapshots.getCount(); ++i)
        {
            const auto& snapshot = session->m_bufferSnapshots[i];
            if (snapshot.buffer.value == handle.value && snapshot.elementType == info.elementType &&
                start >= snapshot.first &&
                uint64_t(start) + count <= uint64_t(snapshot.first) + snapshot.count)
                return i;
        }
        uint64_t bytes = uint64_t(count) * info.hostStride;
        if (bytes > kMaxDebugBufferBytes - session->m_bufferSnapshotBytes)
        {
            fail(
                "Buffer snapshots exceed the 64 MiB limit for this stop; request a smaller range.");
            return -1;
        }
        BufferSnapshot snapshot{handle, info.elementType, start, count, info.hostStride};
        if (SLANG_FAILED(
                interpreter
                    ->readDebugBufferElements(call, handle, start, count, snapshot.bytes, error)))
            return -1;
        session->m_bufferSnapshotBytes += bytes;
        Index result = session->m_bufferSnapshots.getCount();
        session->m_bufferSnapshots.add(std::move(snapshot));
        return result;
    }

    bool dereference(ValueLocation& value)
    {
        const auto* layout = map->findTypeLayout(value.type);
        if (!layout || layout->kind != TypeKind::Address)
            return fail("Cannot dereference a non-pointer value.");
        Address address;
        if (!read(value, &address, sizeof(address)))
            return false;
        value = {layout->valueType, address};
        return true;
    }

    bool followReference(ValueLocation& value)
    {
        auto metadata = map->findType(value.type);
        if (metadata &&
            (metadata->opcode == HGBCOpcode::RefType || metadata->opcode == HGBCOpcode::OutType ||
             metadata->opcode == HGBCOpcode::InOutType ||
             metadata->opcode == HGBCOpcode::BorrowInType))
            return dereference(value);
        return true;
    }

    bool openObject(ValueLocation& value)
    {
        if (value.heapData)
            return true;
        Address handle;
        if (!read(value, &handle, sizeof(handle)))
            return false;
        HGBCId type;
        Address data;
        uint32_t count;
        if (!interpreter->getDebugObject(call, handle, type, data, count) || type != value.type)
            return fail("Cannot read this object (null or no longer live).");
        value.address = data;
        value.offset = 0;
        value.heapData = true;
        value.count = count;
        return true;
    }

    bool member(ValueLocation& value, const String& name)
    {
        if (!followReference(value))
            return false;
        const auto* layout = map->findTypeLayout(value.type);
        if (!layout)
            return fail("No layout for this type.");
        if (layout->kind == TypeKind::HeapObject && !openObject(value))
            return false;
        for (const auto& field : layout->fields)
            if (field.name == name)
            {
                value = {
                    field.type,
                    value.address,
                    value.offset + field.offset,
                    false,
                    0,
                    0,
                    value.snapshot};
                return true;
            }
        if (layout->kind == TypeKind::Vector && name.getLength() == 1)
        {
            static constexpr char lanes[] = "xyzw";
            const char* lane = std::strchr(lanes, name[0]);
            if (lane)
                return element(value, uint32_t(lane - lanes));
        }
        return fail("No such member.");
    }

    bool element(ValueLocation& value, uint32_t index)
    {
        if (!followReference(value))
            return false;
        const auto* layout = map->findTypeLayout(value.type);
        if (!layout)
            return fail("No layout for this type.");
        if (layout->kind == TypeKind::HeapObject &&
            layout->heapKind == HeapObjectKind::StructuredBuffer)
        {
            // A type query resolves the element type without reading any device contents.
            if (typeOnly)
            {
                value = {layout->elementType};
                return true;
            }
            Index snapshotIndex = bufferSnapshot(value, index, 1);
            if (snapshotIndex < 0)
                return false;
            const auto& snapshot = session->m_bufferSnapshots[snapshotIndex];
            value = {
                layout->elementType,
                {},
                (index - snapshot.first) * snapshot.stride,
                false,
                0,
                0,
                snapshotIndex};
            return true;
        }
        if (value.rowWidth)
        {
            if (index >= value.rowWidth)
                return fail("Array index out of bounds.");
            value.offset += index * layout->byteSize;
            value.rowWidth = 0;
            return true;
        }
        uint32_t count = layout->elementCount;
        if (layout->kind == TypeKind::HeapObject &&
            layout->heapKind == HeapObjectKind::RuntimeArray)
        {
            if (!openObject(value))
                return false;
            count = value.count;
        }
        else if (
            layout->kind != TypeKind::Array && layout->kind != TypeKind::Vector &&
            layout->kind != TypeKind::Matrix)
            return fail("This value is not indexable.");
        if (index >= count)
            return fail("Array index out of bounds.");
        value = {
            layout->elementType,
            value.address,
            value.offset + index * layout->elementStride,
            false,
            0,
            layout->kind == TypeKind::Matrix ? layout->numeric.columnCount : 0,
            value.snapshot};
        return true;
    }

    String scalar(const ValueLocation& location, const DebugTypeLayout& layout)
    {
        uint64_t bits = 0;
        if (layout.byteSize > sizeof(bits) || !read(location, &bits, layout.byteSize))
            return "<invalid address>";
        char text[96];
        switch (layout.numeric.scalarType)
        {
        case ScalarType::Bool:
            return bits ? "true" : "false";
        case ScalarType::Float16:
            snprintf(text, sizeof(text), "%.9g", double(HalfToFloat(uint16_t(bits))));
            break;
        case ScalarType::BFloat16:
        case ScalarType::Float32:
            {
                uint32_t word = layout.numeric.scalarType == ScalarType::BFloat16
                                    ? uint32_t(bits) << 16
                                    : uint32_t(bits);
                float number;
                std::memcpy(&number, &word, sizeof(number));
                snprintf(text, sizeof(text), "%.9g", double(number));
                break;
            }
        case ScalarType::Float64:
            {
                double number;
                std::memcpy(&number, &bits, sizeof(number));
                snprintf(text, sizeof(text), "%.17g", number);
                break;
            }
        case ScalarType::Int4:
        case ScalarType::Int8:
        case ScalarType::Int16:
        case ScalarType::Int32:
        case ScalarType::Int64:
            {
                uint32_t width =
                    layout.numeric.scalarType == ScalarType::Int4 ? 4 : layout.byteSize * 8;
                int64_t number = int64_t(bits << (64 - width)) >> (64 - width);
                snprintf(text, sizeof(text), "%lld", (long long)number);
                break;
            }
        case ScalarType::UInt4:
        case ScalarType::UInt8:
        case ScalarType::UInt16:
        case ScalarType::UInt32:
        case ScalarType::UInt64:
            snprintf(text, sizeof(text), "%llu", (unsigned long long)bits);
            break;
        default:
            snprintf(text, sizeof(text), "0x%llx", (unsigned long long)bits);
            break;
        }
        return text;
    }

    /// Opens the concrete payload through the interpreter's live-object check, without trusting
    /// a case tag or treating snapshot bytes as an arbitrary native address.
    bool getErasedPayload(const ValueLocation& value, ValueLocation& payload)
    {
        const auto* layout = map->findTypeLayout(value.type);
        Address handle;
        if (layout->kind == TypeKind::TaggedUnion)
        {
            TaggedUnionValue tagged;
            if (!read(value, &tagged, sizeof(tagged)) || !tagged.caseIndexPlusOne)
                return false;
            handle = tagged.payload;
        }
        else if (!read(value, &handle, sizeof(handle)))
            return false;
        if (!handle)
            return false;
        HGBCId type;
        Address data;
        uint32_t count;
        if (!interpreter->getDebugObject(call, handle, type, data, count))
            return false;
        payload = {type, data};
        return true;
    }

    /// Describes children using the same layouts as expression evaluation. Opening a heap object
    /// here validates its lifetime; expanding it later still uses checked interpreter reads.
    uint32_t childCount(ValueLocation& location, bool& named)
    {
        named = false;
        if (!followReference(location))
            return 0;
        const auto* layout = map->findTypeLayout(location.type);
        if (!layout)
            return 0;
        if (layout->kind == TypeKind::TaggedUnion ||
            (layout->kind == TypeKind::HeapObject && layout->heapKind == HeapObjectKind::Box))
        {
            named = true;
            ValueLocation payload;
            return getErasedPayload(location, payload) ? 1 : 0;
        }
        if (location.rowWidth)
            return location.rowWidth;
        if (layout->kind == TypeKind::Address)
        {
            named = true;
            return 1;
        }
        if (layout->kind == TypeKind::HeapObject)
        {
            if (layout->heapKind == HeapObjectKind::StructuredBuffer)
            {
                Address handle;
                DebugBufferInfo info;
                return bufferInfo(location, handle, info) ? info.elementCount : 0;
            }
            if (layout->heapKind != HeapObjectKind::Class &&
                layout->heapKind != HeapObjectKind::RuntimeArray)
                return 0;
            if (!openObject(location))
                return 0;
        }
        if (layout->fields.getCount())
        {
            named = true;
            return uint32_t(layout->fields.getCount());
        }
        return location.heapData ? location.count : layout->elementCount;
    }

    /// Registers an expandable value without reading its children or any GPU contents.
    void registerChildren(DebugValueInfo& result, ValueLocation location)
    {
        bool named;
        uint32_t count = childCount(location, named);
        if (count)
        {
            result.reference = session->m_nextValueReference++;
            session->m_values.add(result.reference, {call, location});
            (named ? result.namedChildren : result.indexedChildren) = count;
        }
    }

    bool child(ValueLocation& location, uint32_t index, String& name)
    {
        const auto* layout = map->findTypeLayout(location.type);
        SLANG_ASSERT(layout);
        if (layout->kind == TypeKind::TaggedUnion ||
            (layout->kind == TypeKind::HeapObject && layout->heapKind == HeapObjectKind::Box))
        {
            SLANG_ASSERT(index == 0);
            name = "payload";
            ValueLocation payload;
            if (!getErasedPayload(location, payload))
                return false;
            location = payload;
            return true;
        }
        if (layout->kind == TypeKind::Address && !location.rowWidth)
        {
            name = "*";
            return dereference(location);
        }
        if (layout->fields.getCount() && !location.rowWidth)
        {
            name = layout->fields[index].name;
            return member(location, name);
        }
        name = String("[") + String(index) + "]";
        return element(location, index);
    }

    /// Expands a bounded CLI preview or registers a lazy value, depending on the request.
    DebugValueInfo preview(ValueLocation location, const String& name, uint32_t depth = 0)
    {
        DebugValueInfo result;
        result.name = name;
        result.typeName = typeName(location);
        if (!followReference(location))
        {
            result.value = "<invalid address>";
            return result;
        }
        const auto* layout = map->findTypeLayout(location.type);
        if (!layout)
        {
            result.value = "<no layout>";
            return result;
        }
        // Vectors, including synthetic matrix rows, are small scalar tuples. Keep them inline
        // even inside a shallow DAP preview; indexing and swizzles still use the ordinary layout.
        if (location.rowWidth || layout->kind == TypeKind::Vector)
        {
            StringBuilder tuple;
            tuple << '(';
            uint32_t count = location.rowWidth ? location.rowWidth : layout->elementCount;
            for (uint32_t i = 0; i < count; ++i)
            {
                if (i)
                    tuple << ", ";
                ValueLocation lane = location;
                if (!element(lane, i))
                {
                    result.value = "<invalid address>";
                    return result;
                }
                tuple << scalar(lane, *map->findTypeLayout(lane.type));
            }
            tuple << ')';
            result.value = tuple.produceString();
            return result;
        }
        if (layout->kind == TypeKind::TaggedUnion ||
            (layout->kind == TypeKind::HeapObject && layout->heapKind == HeapObjectKind::Box))
        {
            result.value = typeName(location);
            if (layout->kind == TypeKind::TaggedUnion)
            {
                TaggedUnionValue tagged;
                if (!read(location, &tagged, sizeof(tagged)))
                    result.value = "<invalid address>";
                else if (!tagged.caseIndexPlusOne)
                    result.value = result.value + " (uninitialized)";
                else
                    result.value = result.value + " (case = " + String(tagged.caseTag) + ")";
            }
            ValueLocation payload;
            if (getErasedPayload(location, payload))
            {
                result.value = result.value + " <" + typeName(payload) + ">";
                if (lazyChildren)
                    registerChildren(result, location);
                else if (depth < 3)
                    result.children.add(preview(payload, "payload", depth + 1));
            }
            return result;
        }
        if (layout->kind == TypeKind::HeapObject && layout->heapKind != HeapObjectKind::Class &&
            layout->heapKind != HeapObjectKind::RuntimeArray)
        {
            Address handle;
            List<DebugObjectProperty> properties;
            if (!read(location, &handle, sizeof(handle)))
                result.value = "<invalid address>";
            else if (!handle.value)
                result.value = typeName(location) + " (null)";
            else if (!interpreter
                          ->getDebugObjectProperties(call, handle, layout->heapKind, properties))
                result.value = typeName(location) + " (<invalid object>)";
            else
            {
                StringBuilder summary;
                summary << typeName(location);
                if (properties.getCount())
                {
                    summary << " (";
                    for (Index i = 0; i < properties.getCount(); ++i)
                    {
                        if (i)
                            summary << ", ";
                        summary << properties[i].name << " = " << properties[i].value;
                    }
                    summary << ')';
                }
                result.value = summary.produceString();
                if (layout->heapKind == HeapObjectKind::Texture2D && lazyChildren)
                {
                    TextureInfo info;
                    if (textureInfo(location, handle, info) && info.mipCount)
                    {
                        result.reference = session->m_nextValueReference++;
                        result.namedChildren = info.mipCount;
                        session->m_values.add(
                            result.reference,
                            {call, location, StoredValueKind::Texture});
                    }
                }
                if (layout->heapKind == HeapObjectKind::StructuredBuffer)
                {
                    if (lazyChildren)
                        registerChildren(result, location);
                    else if (allowBufferReadback && depth < 3)
                    {
                        bool named;
                        uint32_t count = childCount(location, named);
                        uint32_t previewCount = Math::Min(count, 16u);
                        if (previewCount && bufferSnapshot(location, 0, previewCount) < 0)
                            return result;
                        for (uint32_t i = 0; i < previewCount; ++i)
                        {
                            auto childLocation = location;
                            String childName;
                            if (!child(childLocation, i, childName))
                                break;
                            result.children.add(preview(childLocation, childName, depth + 1));
                        }
                        result.childrenTruncated = count > previewCount;
                    }
                }
            }
            return result;
        }
        if (lazyChildren)
        {
            registerChildren(result, location);
            depth = 3;
        }
        if (layout->kind == TypeKind::Scalar && !location.rowWidth)
            result.value = scalar(location, *layout);
        else if (layout->kind == TypeKind::String)
        {
            String text;
            if (!interpreter->readDebugString(call, location.address, location.offset, text))
                result.value = "<invalid address>";
            else
            {
                StringBuilder escaped;
                escaped << '"';
                for (auto ch : text.getUnownedSlice())
                {
                    if (ch == '\n')
                        escaped << "\\n";
                    else if (ch == '\r')
                        escaped << "\\r";
                    else if (ch == '\t')
                        escaped << "\\t";
                    else if (ch == '"' || ch == '\\')
                        escaped << '\\' << ch;
                    else if (uint8_t(ch) < 32)
                        escaped << '?';
                    else
                        escaped << ch;
                }
                escaped << '"';
                result.value = escaped.produceString();
            }
        }
        else if (layout->kind == TypeKind::Address)
        {
            uint64_t bits;
            if (!read(location, &bits, sizeof(bits)))
                result.value = "<invalid address>";
            else
            {
                char text[32];
                snprintf(text, sizeof(text), "0x%llx", (unsigned long long)bits);
                result.value = text;
            }
        }
        else
        {
            if (layout->kind == TypeKind::HeapObject && !openObject(location))
            {
                result.value = "<invalid address>";
                return result;
            }
            bool named;
            uint32_t count = childCount(location, named);
            result.value = typeName(location);
            if (depth < 3)
            {
                for (uint32_t i = 0; i < count && i < 16; ++i)
                {
                    ValueLocation childLocation = location;
                    String childName;
                    if (!child(childLocation, i, childName))
                        break;
                    result.children.add(preview(childLocation, childName, depth + 1));
                }
                result.childrenTruncated = count > 16;
            }
        }
        return result;
    }
};

/// Parses only read-only variable paths. Function calls and arbitrary evaluation are excluded.
struct DebugSession::ValueExpression
{
public:
    ValueReader& reader;
    const List<const DebugVariableInfo*>& variables;
    const char* current;

    void spaces()
    {
        while (*current == ' ' || *current == '\t')
            ++current;
    }
    bool identifierChar(char c) const
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
               (c >= '0' && c <= '9');
    }
    String identifier()
    {
        spaces();
        const char* start = current;
        while (identifierChar(*current))
            ++current;
        return String(UnownedStringSlice(start, current));
    }
    bool parse(ValueLocation& value)
    {
        spaces();
        if (*current == '*')
        {
            ++current;
            return parse(value) && reader.dereference(value);
        }
        if (*current == '(')
        {
            ++current;
            if (!parse(value))
                return false;
            spaces();
            if (*current++ != ')')
                return reader.fail("Expected ')'.");
        }
        else
        {
            String name = identifier();
            const DebugVariableInfo* found = nullptr;
            for (auto variable : variables)
                if (variable->name == name)
                {
                    found = variable;
                    break;
                }
            if (!found)
                return reader.fail("No such variable in the selected frame.");
            value = {found->type, Address::makeRelativeFrame(found->frameOffset)};
        }
        for (;;)
        {
            spaces();
            if (*current == '.')
            {
                ++current;
                if (!reader.member(value, identifier()))
                    return false;
            }
            else if (current[0] == '-' && current[1] == '>')
            {
                current += 2;
                if (!reader.dereference(value) || !reader.member(value, identifier()))
                    return false;
            }
            else if (*current == '[')
            {
                ++current;
                spaces();
                const char* start = current;
                while (*current >= '0' && *current <= '9')
                    ++current;
                uint32_t index = 0;
                auto parsed = std::from_chars(start, current, index);
                spaces();
                if (parsed.ec != std::errc() || *current++ != ']')
                    return reader.fail("Expected a non-negative constant index followed by ']'.");
                if (!reader.element(value, index))
                    return false;
            }
            else
                return true;
        }
    }
};

SlangResult DebugSession::getVariables(
    Index frame,
    bool arguments,
    List<DebugValueInfo>& values,
    String& error,
    bool expand)
{
    values.clear();
    VariableContext context;
    if (!getVariableContext(frame, context))
    {
        error = "No stopped frame selected.";
        return SLANG_FAIL;
    }
    ValueReader reader{m_interpreter, m_map, context.call};
    reader.session = this;
    reader.lazyChildren = !expand;
    auto variables = getVisibleVariables(context);
    HashSet<String> names;
    for (auto variable : variables)
        if (bool(variable->argumentIndexPlusOne) == arguments && names.add(variable->name))
            values.add(reader.preview(
                {variable->type, Address::makeRelativeFrame(variable->frameOffset)},
                variable->name));
    return SLANG_OK;
}

SlangResult DebugSession::evaluateExpression(
    Index frame,
    const String& expression,
    DebugValueInfo& value,
    String& error,
    bool typeOnly,
    bool expand)
{
    VariableContext context;
    if (!getVariableContext(frame, context))
    {
        error = "No stopped frame selected.";
        return SLANG_FAIL;
    }
    ValueReader reader{m_interpreter, m_map, context.call};
    reader.session = this;
    reader.lazyChildren = !expand;
    auto variables = getVisibleVariables(context);
    reader.typeOnly = typeOnly;
    reader.allowBufferReadback = !typeOnly;
    ValueExpression parser{reader, variables, expression.getBuffer()};
    ValueLocation location;
    if (!parser.parse(location) || *parser.current)
    {
        error = reader.error.getLength() ? reader.error : String("Unsupported expression syntax.");
        return SLANG_FAIL;
    }
    value = {};
    if (typeOnly)
    {
        value.name = expression;
        value.typeName = reader.typeName(location);
    }
    else
        value = reader.preview(location, expression);
    if (reader.error.getLength())
    {
        error = reader.error;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult DebugSession::getValueChildren(
    uint64_t reference,
    uint32_t start,
    uint32_t count,
    List<DebugValueInfo>& values,
    String& error)
{
    values.clear();
    StoredValue stored;
    if (!m_stopped || !m_values.tryGetValue(reference, stored))
    {
        error = "Invalid or expired value reference.";
        return SLANG_FAIL;
    }
    ValueReader reader{m_interpreter, m_map, stored.call};
    reader.session = this;
    reader.lazyChildren = true;
    reader.allowBufferReadback = true;
    if (stored.kind != StoredValueKind::Value)
    {
        Address handle;
        TextureInfo info;
        if (!reader.textureInfo(stored.location, handle, info))
        {
            error = reader.error;
            return SLANG_FAIL;
        }
        uint32_t total = stored.kind == StoredValueKind::Texture ? info.mipCount : 3;
        uint32_t end =
            count ? uint32_t(Math::Min(uint64_t(total), uint64_t(start) + count)) : total;
        for (uint32_t i = start; i < end; ++i)
        {
            DebugValueInfo value;
            if (stored.kind == StoredValueKind::Texture)
            {
                value.name = String("mip[") + String(i) + "]";
                value.typeName = "Texture mip";
                value.value = String(Math::Max(1u, info.width >> i)) + " x " +
                              String(Math::Max(1u, info.height >> i));
                value.reference = m_nextValueReference++;
                value.namedChildren = 3;
                value.isTextureMip = true;
                m_values.add(
                    value.reference,
                    {stored.call, stored.location, StoredValueKind::TextureMip, i});
            }
            else
            {
                value.name = i == 0 ? "width" : i == 1 ? "height" : "format";
                value.typeName = i == 2 ? "String" : "uint";
                value.value =
                    i == 2
                        ? String(rhi::getFormatInfo(info.format).name)
                        : String(Math::Max(1u, (i == 0 ? info.width : info.height) >> stored.mip));
            }
            values.add(std::move(value));
        }
        return SLANG_OK;
    }
    bool named;
    uint32_t total = reader.childCount(stored.location, named);
    uint32_t end = count ? uint32_t(Math::Min(uint64_t(total), uint64_t(start) + count)) : total;
    if (reader.error.getLength())
    {
        error = reader.error;
        return SLANG_FAIL;
    }
    if (start >= end)
        return SLANG_OK;
    const auto* layout = m_map->findTypeLayout(stored.location.type);
    if (layout && layout->kind == TypeKind::HeapObject &&
        layout->heapKind == HeapObjectKind::StructuredBuffer)
    {
        if (end - start > 16384)
        {
            error = "Too many buffer children requested; use a page of at most 16384 elements.";
            return SLANG_FAIL;
        }
        if (reader.bufferSnapshot(stored.location, start, end - start) < 0)
        {
            error = reader.error;
            return SLANG_FAIL;
        }
    }
    for (uint32_t i = start; i < end; ++i)
    {
        auto location = stored.location;
        String name;
        if (!reader.child(location, i, name))
        {
            error = reader.error;
            return SLANG_FAIL;
        }
        values.add(reader.preview(location, name));
    }
    return SLANG_OK;
}
SlangResult DebugSession::exportTextureMip(
    uint64_t reference,
    SequentialByteWriter& destination,
    String& error)
{
    error = {};
    StoredValue stored;
    if (!m_stopped || !m_values.tryGetValue(reference, stored))
    {
        error = "Invalid or expired value reference.";
        return SLANG_FAIL;
    }
    if (stored.kind != StoredValueKind::TextureMip)
    {
        error = "Select a texture mip to export.";
        return SLANG_FAIL;
    }
    ValueReader reader{m_interpreter, m_map, stored.call};
    reader.session = this;
    Address handle;
    TextureInfo info;
    if (!reader.textureInfo(stored.location, handle, info))
    {
        error = reader.error;
        return SLANG_FAIL;
    }
    return m_interpreter
        ->exportDebugTextureMip(stored.call, handle, stored.mip, destination, error);
}
} // namespace Slang::HGVM
