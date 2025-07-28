// slang-serialize-fossil.cpp
#include "slang-serialize-fossil.h"

#include "../core/slang-blob.h"
#include "core/slang-performance-profiler.h"

namespace Slang
{
namespace Fossil
{

//
// SerialWriter
//

SerialWriter::SerialWriter(ChunkBuilder* chunk)
    : _arena(4096)
{
    _initialize(chunk);
}

SerialWriter::SerialWriter(BlobBuilder& blobBuilder)
    : _arena(4096)
{
    auto chunk = blobBuilder.addChunk();
    _initialize(chunk);
}

void SerialWriter::_initialize(ChunkBuilder* chunk)
{
    _blobBuilder = chunk->getParentBlob();

    // The top-level structure consists of a header,
    // and a root value. We will allocate a distinct
    // chunk for each of them, with the header coming
    // first.
    //
    auto headerChunk = chunk;
    auto rootValueChunk = headerChunk->addChunkAfter();

    // We will write the fields of the header chunk manually,
    // although we will use a temporary of type `Fossil::Header`
    // to help make sure we write them with the correct sizes.
    //
    Fossil::Header header;
    memcpy(header.magic, Fossil::Header::kMagic, sizeof(Fossil::Header::kMagic));
    header.totalSizeIncludingHeader = 0;
    header.flags = 0;

    headerChunk->writeData(&header.magic, sizeof(header.magic));
    headerChunk->writeData(
        &header.totalSizeIncludingHeader,
        sizeof(header.totalSizeIncludingHeader));
    headerChunk->writeData(&header.flags, sizeof(header.flags));

    // The main reason we are writing the fields manually is
    // that the last field of the header is a relative pointer
    // to the root-value chunk.
    //
    headerChunk->writeRelativePtr<FossilInt>(rootValueChunk);

    // The root value should always be a variant, and we want to
    // set up to write into it in a reasonable way.
    //
    auto rootPtrLayout = _createLayout(FossilizedValKind::Ptr);
    _state = State(rootPtrLayout, rootValueChunk);

    _pushVariantScope();
}


SerialWriter::~SerialWriter()
{
    _popVariantScope();

    _flush();
}

SerializationMode SerialWriter::getMode()
{
    return SerializationMode::Write;
}

void SerialWriter::handleBool(bool& value)
{
    // A boolean value will be serialized as a full byte.
    uint8_t v = value;
    _writeSimpleValue(FossilizedValKind::Bool, v);
}

void SerialWriter::handleInt8(int8_t& value)
{
    _writeSimpleValue(FossilizedValKind::Int8, value);
}

void SerialWriter::handleInt16(int16_t& value)
{
    _writeSimpleValue(FossilizedValKind::Int16, value);
}

void SerialWriter::handleInt32(Int32& value)
{
    _writeSimpleValue(FossilizedValKind::Int32, value);
}

void SerialWriter::handleInt64(Int64& value)
{
    _writeSimpleValue(FossilizedValKind::Int64, value);
}

void SerialWriter::handleUInt8(uint8_t& value)
{
    _writeSimpleValue(FossilizedValKind::UInt8, value);
}

void SerialWriter::handleUInt16(uint16_t& value)
{
    _writeSimpleValue(FossilizedValKind::UInt16, value);
}

void SerialWriter::handleUInt32(UInt32& value)
{
    _writeSimpleValue(FossilizedValKind::UInt32, value);
}

void SerialWriter::handleUInt64(UInt64& value)
{
    _writeSimpleValue(FossilizedValKind::UInt64, value);
}

void SerialWriter::handleFloat32(float& value)
{
    _writeSimpleValue(FossilizedValKind::Float32, value);
}

void SerialWriter::handleFloat64(double& value)
{
    _writeSimpleValue(FossilizedValKind::Float64, value);
}

void SerialWriter::handleString(String& value)
{
    auto size = value.getLength();
    if (_shouldEmitPotentiallyIndirectValueWithPointerIndirection())
    {
        ChunkBuilder* existingChunk = nullptr;
        _mapStringToChunk.tryGetValue(value, existingChunk);

        // If we found an existing chunk that holds the string
        // value in question, we can re-use it. Also, if the
        // string is empty, we can encode it as a null pointer
        // in this case, so we don't care if we found a chunk
        // or not.
        //
        if (existingChunk || size == 0)
        {
            auto ptrLayout =
                (ContainerLayoutObj*)_reserveDestinationForWrite(FossilizedValKind::Ptr);
            _mergeLayout(ptrLayout->baseLayout, FossilizedValKind::StringObj);

            _commitWrite(ValInfo::relativePtrTo(existingChunk));
            return;
        }
    }

    _pushPotentiallyIndirectValueScope(FossilizedValKind::StringObj);

    auto data = value.getBuffer();
    _writeValueRaw(ValInfo::rawData(data, size + 1, 1));

    auto chunk = _popPotentiallyIndirectValueScope();

    auto rawSize = UInt32(size);
    chunk->addPrefixData(&rawSize, sizeof(rawSize));

    _mapStringToChunk.addIfNotExists(value, chunk);
}

void SerialWriter::beginArray(Scope&)
{
    _pushContainerScope(FossilizedValKind::ArrayObj);
}

void SerialWriter::endArray(Scope&)
{
    _popContainerScope();
}

void SerialWriter::beginDictionary(Scope&)
{
    _pushContainerScope(FossilizedValKind::DictionaryObj);
}

void SerialWriter::endDictionary(Scope&)
{
    _popContainerScope();
}

void SerialWriter::_pushContainerScope(FossilizedValKind kind)
{
    _pushPotentiallyIndirectValueScope(kind);
}

void SerialWriter::_popContainerScope()
{
    auto elementCount = _state.elementCount;
    auto containerChunk = _popPotentiallyIndirectValueScope();

    if (containerChunk)
    {
        auto rawElementCount = UInt32(elementCount);
        containerChunk->addPrefixData(&rawElementCount, sizeof(rawElementCount));
    }
}

bool SerialWriter::hasElements()
{
    return false;
}

void SerialWriter::beginStruct(Scope&)
{
    _pushInlineValueScope(FossilizedValKind::Struct);
}

void SerialWriter::endStruct(Scope&)
{
    _popInlineValueScope();
}

void SerialWriter::beginVariant(Scope&)
{
    _pushVariantScope();
    _pushInlineValueScope(FossilizedValKind::Struct);
}

void SerialWriter::endVariant(Scope&)
{
    _popInlineValueScope();
    _popVariantScope();
}

void SerialWriter::handleFieldKey(char const* name, Int index)
{
    // For now we are ignoring field keys, and treating
    // structs as basically equivalent to tuples.
    SLANG_UNUSED(name);
    SLANG_UNUSED(index);
}

void SerialWriter::beginTuple(Scope&)
{
    _pushInlineValueScope(FossilizedValKind::Tuple);
}

void SerialWriter::endTuple(Scope&)
{
    _popInlineValueScope();
}

void SerialWriter::beginOptional(Scope&)
{
    _pushIndirectValueScope(FossilizedValKind::OptionalObj);
}

void SerialWriter::endOptional(Scope&)
{
    _popIndirectValueScope();
}

void SerialWriter::handleSharedPtr(void*& value, SerializerCallback callback, void* context)
{
    // Because we are writing, we only care about the
    // pointer that is already present in `value`.
    //
    void* liveObjectPtr = value;

    // The first special case we check for is a null pointer,
    // which we can serialize as an inline value.
    //
    if (liveObjectPtr == nullptr)
    {
        _writeNull();
        return;
    }

    // Next, we check to see if we have encountered this
    // pointer before, in which case we've already allocated
    // an index for it in the object definition list, and
    // we can simply write a reference to that object.
    //
    if (auto found = _mapLiveObjectPtrToFossilizedObject.tryGetValue(liveObjectPtr))
    {
        auto fossilizedObject = *found;

        _reserveDestinationForWrite(fossilizedObject->ptrLayout);
        _commitWrite(ValInfo::relativePtrTo(fossilizedObject->chunk));

        return;
    }

    auto ptrLayout = _reserveDestinationForWrite(FossilizedValKind::Ptr);
    auto chunk = _blobBuilder->addChunk();

    auto fossilizedObject = new (_arena) FossilizedObjectInfo();
    fossilizedObject->chunk = chunk;
    fossilizedObject->ptrLayout = ptrLayout;
    fossilizedObject->liveObjectPtr = liveObjectPtr;
    fossilizedObject->callback = callback;
    fossilizedObject->context = context;

    _fossilizedObjects.add(fossilizedObject);
    _mapLiveObjectPtrToFossilizedObject.add(liveObjectPtr, fossilizedObject);

    _commitWrite(ValInfo::relativePtrTo(chunk));
}

void SerialWriter::handleUniquePtr(void*& value, SerializerCallback callback, void* context)
{
    // We treat all pointers as shared pointers, because there isn't really
    // an optimized representation we would want to use for the unique case.
    //
    handleSharedPtr(value, callback, context);
}

void SerialWriter::handleDeferredObjectContents(
    void* valuePtr,
    SerializerCallback callback,
    void* context)
{
    // Because we are already deferring writing of the *entirety* of
    // an object's members as part of how `handleSharedPtr()` works,
    // we don't need to implement deferral at this juncture.
    //
    // (In practice the `handleDeferredObjectContents()` operation is
    // more for the benefit of reading than writing).
    //
    callback(valuePtr, this, context);
}

SerialWriter::LayoutObj* SerialWriter::_createSimpleLayout(FossilizedValKind kind)
{
    switch (kind)
    {
    case FossilizedValKind::Bool:
    case FossilizedValKind::Int8:
    case FossilizedValKind::UInt8:
        return new (_arena) SimpleLayoutObj(kind, 1);

    case FossilizedValKind::Int16:
    case FossilizedValKind::UInt16:
        return new (_arena) SimpleLayoutObj(kind, 2);

    case FossilizedValKind::Int32:
    case FossilizedValKind::UInt32:
    case FossilizedValKind::Float32:
        return new (_arena) SimpleLayoutObj(kind, 4);

    case FossilizedValKind::Int64:
    case FossilizedValKind::UInt64:
    case FossilizedValKind::Float64:
        return new (_arena) SimpleLayoutObj(kind, 8);

    case FossilizedValKind::StringObj:
        return new (_arena) SimpleLayoutObj(kind);

    default:
        SLANG_UNEXPECTED("unhandled case");
        UNREACHABLE_RETURN(nullptr);
    }
}

SerialWriter::LayoutObj* SerialWriter::_createLayout(FossilizedValKind kind)
{
    switch (kind)
    {
    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::DictionaryObj:
        return new (_arena) ContainerLayoutObj(kind, nullptr);

    case FossilizedValKind::Ptr:
        return new (_arena) ContainerLayoutObj(kind, nullptr, sizeof(FossilInt), sizeof(FossilInt));

    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
        return new (_arena) RecordLayoutObj(kind);

    case FossilizedValKind::VariantObj:
        // A variant is being treated like a container in this context,
        // because it wants to be able to track the layout of what it
        // ended up holding...
        //
        return new (_arena) ContainerLayoutObj(kind, nullptr);

    case FossilizedValKind::Bool:
    case FossilizedValKind::Int8:
    case FossilizedValKind::Int16:
    case FossilizedValKind::Int32:
    case FossilizedValKind::Int64:
    case FossilizedValKind::UInt8:
    case FossilizedValKind::UInt16:
    case FossilizedValKind::UInt32:
    case FossilizedValKind::UInt64:
    case FossilizedValKind::Float32:
    case FossilizedValKind::Float64:
    case FossilizedValKind::StringObj:
        {
            if (auto found = _simpleLayouts.tryGetValue(kind))
                return *found;

            auto layout = _createSimpleLayout(kind);
            _simpleLayouts.add(kind, layout);
            return layout;
        }

    default:
        SLANG_UNEXPECTED("unhandled case");
        UNREACHABLE_RETURN(nullptr);
    }
}

SerialWriter::LayoutObj* SerialWriter::_mergeLayout(LayoutObj*& dst, FossilizedValKind kind)
{
    if (!dst)
    {
        dst = _createLayout(kind);
    }

    if (dst->kind != kind)
    {
        SLANG_UNEXPECTED("type mismatch during serialization");
    }

    // As a special case, if the right-hand-side is a variant,
    // then we want to have a unique layout object for each
    // instance.
    //
    if (kind == FossilizedValKind::VariantObj)
    {
        auto src = _createLayout(kind);
        return src;
    }

    return dst;
}

void SerialWriter::_mergeLayout(LayoutObj*& dst, LayoutObj* src)
{
    if (dst == src)
        return;

    if (!src)
        return;

    if (!dst)
    {
        dst = src;
        return;
    }

    _mergeLayout(dst, src->getKind());

    switch (src->getKind())
    {
    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::Ptr:
        {
            auto dstContainer = (ContainerLayoutObj*)dst;
            auto srcContainer = (ContainerLayoutObj*)src;
            _mergeLayout(dstContainer->baseLayout, srcContainer->baseLayout);
        }
        break;

    case FossilizedValKind::StringObj:
        break;

    case FossilizedValKind::VariantObj:
        // Recursive merging should not be applied to variants;
        // each variant is unique until later deduplication.
        break;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

SerialWriter::RecordLayoutObj::FieldInfo& SerialWriter::_getOrAddField(
    RecordLayoutObj* recordLayout,
    Index index)
{
    // Note: we are doing all the allocation for `LayoutObj`s from
    // an arena, so that we don't have to worry about managing
    // their lifetimes carefully.
    //
    // One place where that is a bit tedious is handling the storage
    // for the array of fields for a record.
    //
    // TODO(tfoley): see if there's allocator support on `List<T>`
    // or similar, so that it can be made to just use the arena.

    SLANG_ASSERT(recordLayout);
    SLANG_ASSERT(index >= 0);

    if (index < recordLayout->fieldCount)
        return recordLayout->fields[index];

    SLANG_ASSERT(index == recordLayout->fieldCount);

    if (index >= recordLayout->fieldCapacity)
    {
        if (recordLayout->fieldCapacity == 0)
            recordLayout->fieldCapacity = 16;

        while (index >= recordLayout->fieldCapacity)
        {
            recordLayout->fieldCapacity = (recordLayout->fieldCapacity * 3) >> 1;
        }

        auto newFields = new (_arena) RecordLayoutObj::FieldInfo[recordLayout->fieldCapacity];
        for (Index i = 0; i < recordLayout->fieldCount; ++i)
            newFields[i] = recordLayout->fields[i];
        recordLayout->fields = newFields;
    }

    recordLayout->fields[recordLayout->fieldCount++] = RecordLayoutObj::FieldInfo();
    return recordLayout->fields[index];
}

SerialWriter::ValInfo SerialWriter::ValInfo::rawData(void const* data, Size size, Size alignment)
{
    ValInfo val(Kind::RawData);
    val.data.ptr = data;
    val.data.size = size;
    val.data.alignment = alignment;
    return val;
}

SerialWriter::ValInfo SerialWriter::ValInfo::relativePtrTo(ChunkBuilder* targetChunk)
{
    ValInfo val(Kind::RelativePtr);
    val.chunk = targetChunk;
    return val;
}

SerialWriter::ValInfo SerialWriter::ValInfo::contentsOf(ChunkBuilder* chunk)
{
    ValInfo val(Kind::ContentsOfChunk);
    val.chunk = chunk;
    return val;
}

Size SerialWriter::ValInfo::getAlignment() const
{
    switch (kind)
    {
    case Kind::RelativePtr:
        return sizeof(FossilInt);

    case Kind::ContentsOfChunk:
        return chunk->getAlignment();

    case Kind::RawData:
        return data.alignment;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

void SerialWriter::_pushInlineValueScope(FossilizedValKind kind)
{
    auto layout = _reserveDestinationForWrite(kind);
    _pushState(layout);
}

void SerialWriter::_popInlineValueScope()
{
    auto layout = _state.layout;
    auto chunk = _state.chunk;

    if (chunk)
    {
        if (layout->size == 0)
        {
            layout->size = chunk->getContentSize();
        }
        SLANG_ASSERT(layout->size == chunk->getContentSize());
    }

    _popState();

    _commitWrite(ValInfo::contentsOf(chunk));
}

void SerialWriter::_pushVariantScope()
{
    _pushPotentiallyIndirectValueScope(FossilizedValKind::VariantObj);
}

void SerialWriter::_popVariantScope()
{
    SLANG_ASSERT(_state.layout);
    SLANG_ASSERT(_state.layout->kind == FossilizedValKind::VariantObj);
    auto variantLayout = (ContainerLayoutObj*)_state.layout;
    auto valueLayout = variantLayout->baseLayout;
    SLANG_ASSERT(valueLayout);

    auto variantChunk = _popPotentiallyIndirectValueScope();

    // The key feature of a variant is that it carries its own
    // layout information.
    //
    // We need to insert a pointer to the serialized form
    // of the layout information for the element type as a header
    // *before* the content.
    //
    // The first step there is to turn the element layout into
    // a handle such that we can write a relative pointer to it.
    //

    VariantInfo variantInfo;
    variantInfo.layout = valueLayout;
    variantInfo.chunk = variantChunk;
    _variants.add(variantInfo);
}


void SerialWriter::_pushPotentiallyIndirectValueScope(FossilizedValKind kind)
{
    if (_shouldEmitPotentiallyIndirectValueWithPointerIndirection())
    {
        _pushIndirectValueScope(kind);
    }
    else
    {
        _pushInlineValueScope(kind);
    }
}

ChunkBuilder* SerialWriter::_popPotentiallyIndirectValueScope()
{
    // TODO(tfoley): Try to make this function just be a simple
    // conditional to select between the functions for the
    // indirect and inline cases.

    auto valueChunk = _state.chunk;
    _popState();

    if (_shouldEmitPotentiallyIndirectValueWithPointerIndirection())
    {
        return _writeKnownIndirectValueSharedLogic(valueChunk);
    }
    else
    {
        _commitWrite(ValInfo::contentsOf(valueChunk));
        return _state.chunk;
    }
}

void SerialWriter::_pushIndirectValueScope(FossilizedValKind kind)
{
    auto ptrLayout = (ContainerLayoutObj*)_reserveDestinationForWrite(FossilizedValKind::Ptr);
    auto valueLayout = _mergeLayout(ptrLayout->baseLayout, kind);

    _pushState(valueLayout);
}

void SerialWriter::_popIndirectValueScope()
{
    auto valueChunk = _state.chunk;
    _popState();

    _writeKnownIndirectValueSharedLogic(valueChunk);
}

ChunkBuilder* SerialWriter::_writeKnownIndirectValueSharedLogic(ChunkBuilder* valueChunk)
{
    if (!valueChunk)
    {
        _commitWrite(ValInfo::relativePtrTo(nullptr));
        return nullptr;
    }

    _blobBuilder->addChunk(valueChunk);

    _commitWrite(ValInfo::relativePtrTo(valueChunk));
    return valueChunk;
}


void SerialWriter::_pushState(LayoutObj* layout)
{
    _stack.add(_state);
    _state = State(layout);
}

void SerialWriter::_popState()
{
    SLANG_ASSERT(_stack.getCount() != 0);
    _state = _stack.getLast();
    _stack.removeLast();
}

void SerialWriter::_ensureChunkExists()
{
    if (_state.chunk != nullptr)
        return;

    _state.chunk = _blobBuilder->createUnparentedChunk();
}

void SerialWriter::_writeValueRaw(ValInfo const& val)
{
    switch (val.kind)
    {
    case ValInfo::Kind::RawData:
        if (val.data.size == 0)
            return;
        _ensureChunkExists();
        _state.chunk->writePaddingToAlignTo(val.data.alignment);
        _state.chunk->writeData(val.data.ptr, val.data.size);
        break;

    case ValInfo::Kind::RelativePtr:
        _ensureChunkExists();
        _state.chunk->writeRelativePtr<FossilInt>(val.chunk);
        break;

    case ValInfo::Kind::ContentsOfChunk:
        {
            if (!val.chunk)
                return;

            if (!_state.chunk)
            {
                _state.chunk = val.chunk;
            }
            else
            {
                _state.chunk->addContentsOf(val.chunk);
            }
        }
        break;

    default:
        SLANG_UNEXPECTED("unknown Fossil::SerialWriter::ValInfo::Kind");
        break;
    }
}

bool SerialWriter::_shouldEmitPotentiallyIndirectValueWithPointerIndirection()
{
    switch (_state.layout->getKind())
    {
    default:
        return true;

    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::Ptr:
        return false;
    }
}

SerialWriter::LayoutObj*& SerialWriter::_reserveDestinationForWrite()
{
    switch (_state.layout->getKind())
    {
    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
        {
            auto recordLayout = (RecordLayoutObj*)_state.layout;
            auto elementIndex = _state.elementCount;
            auto& elementLayout = _getOrAddField(recordLayout, elementIndex).layout;
            return elementLayout;
        }
        break;

    case FossilizedValKind::Ptr:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::VariantObj:
        {
            auto containerLayout = (ContainerLayoutObj*)_state.layout;
            auto& elementLayout = containerLayout->baseLayout;
            return elementLayout;
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

SerialWriter::LayoutObj* SerialWriter::_reserveDestinationForWrite(FossilizedValKind srcKind)
{
    return _mergeLayout(_reserveDestinationForWrite(), srcKind);
}

SerialWriter::LayoutObj* SerialWriter::_reserveDestinationForWrite(LayoutObj* srcLayout)
{
    SLANG_ASSERT(srcLayout != nullptr);
    _mergeLayout(_reserveDestinationForWrite(), srcLayout);
    return srcLayout;
}

void SerialWriter::_commitWrite(ValInfo const& val)
{
    auto outerKind = _state.layout->getKind();
    switch (outerKind)
    {
    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
        {
            auto recordLayout = (RecordLayoutObj*)_state.layout;
            auto elementIndex = _state.elementCount++;
            auto& fieldInfo = _getOrAddField(recordLayout, elementIndex);

            Size fieldOffset = 0;
            if (elementIndex != 0)
            {
                auto chunk = _state.chunk;
                chunk->writePaddingToAlignTo(val.getAlignment());

                fieldOffset = chunk->getContentSize();
            }
            fieldInfo.offset = fieldOffset;

            _writeValueRaw(val);
        }
        break;

    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::Ptr:
    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::VariantObj:
        {
            auto elementIndex = _state.elementCount++;

            switch (outerKind)
            {
            case FossilizedValKind::OptionalObj:
            case FossilizedValKind::Ptr:
                if (elementIndex > 0)
                {
                    SLANG_UNEXPECTED(
                        "error during serialization: optional with more than one value inside!!");
                }
                break;

            default:
                break;
            }

            _writeValueRaw(val);
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

void SerialWriter::_writeSimpleValue(
    FossilizedValKind kind,
    void const* data,
    size_t size,
    size_t alignment)
{
    auto layout = _reserveDestinationForWrite(kind);
    SLANG_ASSERT(layout->size == size);
    SLANG_ASSERT(layout->alignment == alignment);
    _commitWrite(ValInfo::rawData(data, size, alignment));
}

void SerialWriter::_writeNull()
{
    RelativePtrOffset offset = 0;
    _writeSimpleValue(FossilizedValKind::Ptr, offset);
}

void SerialWriter::_flush()
{
    while (_writtenObjectDefinitionCount < _fossilizedObjects.getCount())
    {
        auto objectIndex = _writtenObjectDefinitionCount++;
        auto fossilizedObject = _fossilizedObjects[objectIndex];

        SLANG_ASSERT(fossilizedObject->liveObjectPtr);

        _state = State(fossilizedObject->ptrLayout, fossilizedObject->chunk);

        fossilizedObject->callback(
            &fossilizedObject->liveObjectPtr,
            this,
            fossilizedObject->context);
    }

    // Once we've written out all the payload data, we can start to work on
    // serializing layout information for all the variant values that were
    // written.
    //
    for (auto variantInfo : _variants)
    {
        auto layoutChunk = _getOrCreateChunkForLayout(variantInfo.layout);
        variantInfo.chunk->addPrefixRelativePtr<FossilInt>(layoutChunk);
    }
}

ChunkBuilder* SerialWriter::_getOrCreateChunkForLayout(LayoutObj* layout)
{
    if (!layout)
        return nullptr;

    // We start by looking for an existing chunk for `layout`,
    // which would be cached on the object itself.
    //
    if (auto existingChunk = layout->chunk)
        return existingChunk;

    // Next we look for an existing chunk that matches the
    // structure of `layout`.
    //
    LayoutObjKey key = {layout};
    if (auto found = _mapLayoutObjToChunk.tryGetValue(key))
    {
        auto existingChunk = *found;
        layout->chunk = existingChunk;
        return existingChunk;
    }

    // If no existing layout has been written to a chunk,
    // then we'll create one.
    //
    auto chunk = _blobBuilder->addChunk();
    layout->chunk = chunk;
    _mapLayoutObjToChunk.add(key, chunk);

    auto kind = layout->getKind();
    auto rawKind = UInt32(kind);
    chunk->writeData(&rawKind, sizeof(rawKind));

    switch (kind)
    {
    default:
        break;

    case FossilizedValKind::Ptr:
    case FossilizedValKind::OptionalObj:
        {
            auto containerLayout = (ContainerLayoutObj*)layout;
            auto elementLayout = containerLayout->baseLayout;
            auto elementLayoutChunk = _getOrCreateChunkForLayout(elementLayout);
            chunk->writeRelativePtr<FossilInt>(elementLayoutChunk);
        }
        break;

    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::DictionaryObj:
        {
            auto containerLayout = (ContainerLayoutObj*)layout;
            auto elementLayout = containerLayout->baseLayout;
            auto elementLayoutChunk = _getOrCreateChunkForLayout(elementLayout);
            chunk->writeRelativePtr<FossilInt>(elementLayoutChunk);

            UInt32 elementStride = 0;
            if (elementLayout)
            {
                elementStride =
                    UInt32(roundUpToAlignment(elementLayout->size, elementLayout->alignment));
                SLANG_ASSERT(elementStride != 0);
            }
            chunk->writeData(&elementStride, sizeof(elementStride));
        }
        break;

    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
        {
            auto recordLayout = (RecordLayoutObj*)layout;

            auto fieldCount = UInt32(recordLayout->fieldCount);
            chunk->writeData(&fieldCount, sizeof(fieldCount));

            for (Index i = 0; i < fieldCount; ++i)
            {
                auto& field = recordLayout->fields[i];
                auto fieldLayoutChunk = _getOrCreateChunkForLayout(field.layout);
                chunk->writeRelativePtr<FossilInt>(fieldLayoutChunk);

                auto fieldOffset = UInt32(field.offset);
                chunk->writeData(&fieldOffset, sizeof(fieldOffset));

                if (i != 0)
                {
                    // Make sure that all but the first field have
                    // a non-zero offset, to validate that offsets
                    // are being comptued at all.
                    //
                    SLANG_ASSERT(fieldOffset != 0);
                }
            }
        }
        break;
    }

    return chunk;
}

bool SerialWriter::LayoutObjKey::operator==(LayoutObjKey const& that) const
{
    if (obj == that.obj)
        return true;

    if (!obj || !that.obj)
        return false;

    SLANG_ASSERT(obj && that.obj);

    if (obj->kind != that.obj->kind)
        return false;

    switch (obj->kind)
    {
    default:
        break;

    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::Ptr:
        {
            auto thisContainer = (ContainerLayoutObj*)obj;
            auto thatContainer = (ContainerLayoutObj*)that.obj;

            LayoutObjKey thisElement = thisContainer->baseLayout;
            LayoutObjKey thatElement = thatContainer->baseLayout;

            if (thisElement != thatElement)
                return false;
        }
        break;

    case FossilizedValKind::Tuple:
    case FossilizedValKind::Struct:
        {
            auto thisRecord = (RecordLayoutObj*)obj;
            auto thatRecord = (RecordLayoutObj*)that.obj;

            if (thisRecord->fieldCount != thatRecord->fieldCount)
                return false;

            auto fieldCount = thisRecord->fieldCount;
            for (Index i = 0; i < fieldCount; ++i)
            {
                auto thisField = thisRecord->fields[i];
                auto thatField = thatRecord->fields[i];

                if (thisField.offset != thatField.offset)
                    return false;

                LayoutObjKey thisFieldLayout = thisField.layout;
                LayoutObjKey thatFieldLayout = thatField.layout;

                if (thisFieldLayout != thatFieldLayout)
                    return false;
            }
        }
        break;
    }

    return true;
}

bool SerialWriter::LayoutObjKey::operator!=(LayoutObjKey const& that) const
{
    return !(*this == that);
}

HashCode64 SerialWriter::LayoutObjKey::getHashCode() const
{
    Hasher hasher;
    hashInto(hasher);
    return hasher.getResult();
}

void SerialWriter::LayoutObjKey::hashInto(Hasher& hasher) const
{
    if (!obj)
    {
        hasher.hashValue(obj);
        return;
    }

    hasher.hashValue(obj->kind);

    switch (obj->kind)
    {
    default:
        break;

    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::Ptr:
        {
            auto container = (ContainerLayoutObj*)obj;

            LayoutObjKey(container->baseLayout).hashInto(hasher);
        }
        break;

    case FossilizedValKind::Tuple:
    case FossilizedValKind::Struct:
        {
            auto record = (RecordLayoutObj*)obj;

            auto fieldCount = record->fieldCount;
            hasher.hashValue(record->fieldCount);

            for (Index i = 0; i < fieldCount; ++i)
            {
                auto& field = record->fields[i];
                hasher.hashValue(field.offset);
                LayoutObjKey(field.layout).hashInto(hasher);
            }
        }
        break;
    }
}


//
// SerialReader
//

SerialReader::SerialReader(
    ReadContext& context,
    Fossil::AnyValPtr valPtr,
    InitialStateType initialState)
    : _context(context)
{
    // We track the number of active `SerialReader`s that
    // are working with the same `ReadContext`, and will
    // make use of this count in the destructor below.
    //
    context._readerCount++;

    switch (initialState)
    {
    case InitialStateType::Root:
        _state.type = State::Type::Object;
        break;

    case InitialStateType::PseudoPtr:
        _state.type = State::Type::PseudoPtr;
        break;
    }

    _state.dataCursor = valPtr.getDataPtr();
    _state.layoutCursor = valPtr.getLayout();
    _state.remainingValueCount = 1;
}

SerialReader::~SerialReader()
{
    // If an application is designed to perform something
    // like on-demand deserialization, it may create
    // additional `SerialReader`s attached to the same
    // `ReadContext`, potentially even in the body of a
    // callback that was invoked by an operation on another
    // `SerialReader` further up the stack.
    //
    // If we were to track the deferred actions that get
    // enqueued on a per-`SerialReader` basis, and then
    // flush them when the given `SerialReader` is destructed,
    // it could potentially lead to very deep call stacks.
    //
    // Instead, we track a single list of deferred actions
    // on the `ReadContext`, which means that we need to
    // figure out when to actually flush that list.
    //
    // What is implemented here is a "last one out shuts the door"
    // policy. When a `SerialReader` is being destroyed, before
    // it decrements the count on the shared `ReadContext`, it
    // checks to see if it is the last remaining `SerialReader`,
    // in which case it takes responsibility for flushing the deferred
    // actions that were enqueued by *all* of the readers.
    //
    // Note that the ordering here is critical: we check whether
    // we are the last reader and, if so, perform the `_flush()`
    // operation all *before* decrementing the counter. If we
    // were to decrement the count before invoking `_flush()`
    // then any nested `SerialReader`s that get created by the
    // deferred actions would (incorrectly) believe themselves
    // to be the "last one out" and try to perform their own
    // `flush()`, which could quickly lead to unbounded
    // recursion.
    //
    if (_context._readerCount == 1)
    {
        _flush();
    }
    _context._readerCount--;
}

void SerialReader::flush()
{
    _flush();
}

void SerialReader::beginVariant(Scope& scope)
{
    auto valPtr = _readPotentiallyIndirectValPtr();
    if (auto variantPtr = as<FossilizedVariantObj>(valPtr))
    {
        auto contentValPtr = getVariantContentPtr(variantPtr);
        valPtr = contentValPtr;
    }
    auto recordPtr = expectNonNullValOfType<FossilizedRecordVal>(valPtr);

    _pushRecordState(scope, recordPtr);
}

void SerialReader::handleSharedPtr(void*& value, SerializerCallback callback, void* context)
{
    Fossil::AnyValPtr valPtr = _readValPtr();

    Fossil::AnyValPtr targetValPtr;
    if (_state.type == State::Type::PseudoPtr)
    {
        // TODO(tfoley): Having to include the `PseudoPtr` case here
        // is frustrating, because it was only introduced to deal
        // with a wrinkle related to on-demand AST deserialization,
        // and ideally shouldn't be needed at all.

        targetValPtr = valPtr;
    }
    else
    {
        auto ptrPtr = expectNonNullValOfType<FossilizedPtr<void>>(valPtr);
        targetValPtr = ptrPtr->getTargetValPtr();
    }

    // The logic here largely mirrors what appears in
    // `SerialWriter::handleSharedPtr`.
    //
    // We first check for an explicitly written null pointer.
    // If we find one our work is very easy.
    //
    if (!targetValPtr)
    {
        value = nullptr;
        return;
    }

    // Now we need to check if we've previously read in
    // a reference to the same object.
    //
    if (auto found = _context.mapFossilizedObjectPtrToObjectInfo.tryGetValue(targetValPtr.get()))
    {
        auto objectInfo = *found;

        // We've seen this object before, although it
        // is still possible that we are in the middle
        // of reading it as part of an invocation
        // of `handleSharedPtr()` further up the call
        // stack.
        //
        // If a non-nullpointer value has already been
        // written into the `objectInfo`, then that means
        // the callback that was run for the prior (or
        // in-flight) read operation has already allocated
        // or found an object and written it out.
        // In that case we will trust the value.
        //
        if (objectInfo->resurrectedObjectPtr == nullptr)
        {
            // It is possible that the pointer is null because
            // the callback that was invoked explicitly *chose*
            // to yield a null pointer (perhaps the application
            // is choosing not to deserialize some optional
            // piece of state).
            //
            // However, if there is still a callback in-flight
            // to read this object, and the pointer is null,
            // then we have reached a circular reference,
            // and need to signal an error.
            //
            if (objectInfo->state == ObjectState::ReadingInProgress)
            {
                SLANG_UNEXPECTED("circularity detected in fossil deserialization");
            }
        }
        value = objectInfo->resurrectedObjectPtr;
        return;
    }

    // At this point we are reading a reference to an
    // object index that has not yet been read at all.
    //
    auto objectInfo = RefPtr(new ObjectInfo());
    _context.mapFossilizedObjectPtrToObjectInfo.add(targetValPtr.get(), objectInfo);

    objectInfo->fossilizedObjectPtr = targetValPtr;

    // We cannot return from this function until we have
    // stored a pointer into `value`, to represent the
    // deserialized object.
    //
    // Thus we will set ourselves up to start reading
    // from the relevant object definition, and invoke
    // the callback that was passed in.
    //
    // Calling into user-defined serialization logic from
    // within this function creates the possibility of
    // unbounded/infinite recursion, so it is vital that
    // the user is properly using `deferSerializeObjectContents()`
    // to delay reading data that isn't immediately
    // necessary.
    //
    // We will still set the `objectInfo.state` to reflect
    // this in-flight operation so that we can detect
    // a cirularity if one occurs at runtime.
    //
    objectInfo->state = ObjectState::ReadingInProgress;

    // We save/restore the current cursor around
    // the callback, because we need to be able
    // to return to the current state to continue
    // reading whatever comes after the pointer
    // we were invoked to read.
    //
    Scope callbackScope;
    _pushState(callbackScope);
    _state.type = State::Type::Object;
    _state.dataCursor = targetValPtr.getDataPtr();
    _state.layoutCursor = targetValPtr.getLayout();
    _state.remainingValueCount = 1;

    // Note that we are passing the address of `objectInfo.ptr`,
    // and `objectInfo` is a reference to an element of the
    // `_objects` array. Thus whenever the `callback` stores
    // a pointer into that output parameter, the value it writes
    // will automatically be visible to any subsequent calls
    // to `handleSharedPtr()`, even if they occur before
    // `callback` returns.
    //
    // Thus a "true" circularity can only occur if the callback
    // recursively reads a reference to the same object again
    // *before* it allocates the in-memory representation of
    // that objects and stores a pointer to it into the output
    // parameter.
    //
    callback(&objectInfo->resurrectedObjectPtr, this, context);

    _popState(callbackScope);

    objectInfo->state = ObjectState::ReadingComplete;

    value = objectInfo->resurrectedObjectPtr;
}

void SerialReader::handleDeferredObjectContents(
    void* valuePtr,
    SerializerCallback callback,
    void* context)
{
    // Unlike the case in `SerialWriter::handleDeferredObjectContents()`,
    // we very much *do* want to delay invoking the callback until later.
    //
    // There is a kind of symmetry going on, where the writer delays the
    // callback passed to `handleSharedPtr()`, but *not* the callback
    // passed to `handleDeferredObjectContents()`, while the reader
    // does the opposite: immediately calls the callback in `handleSharedPtr()`
    // but delays calling it here.

    // We make sure to save the current `_cursor` value along with
    // the arguments that will be passed into the callback, so that
    // we can restore the reader to this state before invoking
    // the callbak in `_flush()`.

    DeferredAction deferredAction;
    deferredAction.savedState = _state;
    deferredAction.resurrectedObjectPtr = valuePtr;
    deferredAction.callback = callback;
    deferredAction.context = context;

    _context._deferredActions.add(deferredAction);
}

void SerialReader::_flush()
{
    // We need to flush any actions that were deferred
    // and are still pending.
    //
    while (_context._deferredActions.getCount() != 0)
    {
        // TODO: For simplicity we are using the `_deferredActions`
        // array as a stack (LIFO), but it would be good to
        // check whether there is a menaingful difference in how
        // large the array would need to grow for a FIFO vs. LIFO,
        // and pick the better option.
        //
        auto deferredAction = _context._deferredActions.getLast();
        _context._deferredActions.removeLast();

        _state = deferredAction.savedState;
        deferredAction.callback(deferredAction.resurrectedObjectPtr, this, deferredAction.context);
    }
}

void SerialReader::_advanceCursor()
{
    auto dataPtr = _state.dataCursor;

    // The state also tracks the number of values that
    // can still be read in the current scope. This value
    // is used both to drive the loop when an application
    // is reading a collection, and as a validation check
    // here.
    //
    auto remainingValueCount = _state.remainingValueCount;
    SLANG_SERIALIZE_FOSSIL_VALIDATE(remainingValueCount > 0);

    // At minimum, we need to update the state to reflect
    // the new number of values that remain.
    //
    remainingValueCount--;
    _state.remainingValueCount = remainingValueCount;

    // At this point, there is no need to update the
    // state further, unless values remain. While
    // we expect the "no values remain" case to be
    // the less common one, we still guard all of
    // this logic because we already need to branch
    // to handle the case of record fields.
    //
    if (remainingValueCount != 0)
    {
        // The primary task in this case is to update
        // the data pointer to point to the next value
        // in the stream.
        //
        auto nextDataPtr = (char*)dataPtr;

        // If we are reading from a record (struct or tuple)
        // then we will have an additional piece of state
        // indicating the field that the data cursor was
        // pointing at.
        //
        if (auto fieldInfoPtr = _state.fieldCursor)
        {
            // We know, because `remainingValueCount` is non-zero,
            // that there is at least one more field after this
            // one, so we can increment the `fieldInfoPtr` to
            // get a pointer to the next field layout in the
            // record layout.
            //
            auto nextFieldInfoPtr = fieldInfoPtr + 1;

            // The layout information for a record type stores
            // the offset of each field, so we can compute
            // the relative offset between two fields as the
            // difference between their offsets in the layout.
            //
            auto offsetToNextField = nextFieldInfoPtr->offset - fieldInfoPtr->offset;

            // Adding that relative offset to the data pointer
            // will navigate us to the next field.
            //
            nextDataPtr += offsetToNextField;
            _state.fieldCursor = nextFieldInfoPtr;

            // The info for the next field stores a (relative)
            // pointer to its data layout, so we can write
            // that into our state so that it is available
            // for the next read operation.

            _state.layoutCursor = nextFieldInfoPtr->layout;
        }
        else
        {
            // The other case where more than one value can
            // be read in the same state is when a collection
            // is being read. In that case the setup logic
            // will have already stored the stride between
            // elements into the state, and we can use that
            // to increment the data pointer.
            //
            // (In the container case the layout pointer doesn't
            // need to change, since all of the elements are
            // required to have the same layout).
            //
            auto dataStride = _state.dataStride;
            nextDataPtr += dataStride;
        }

        _state.dataCursor = nextDataPtr;
    }
}

SLANG_FORCE_INLINE Fossil::AnyValPtr SerialReader::_readIndirectValPtr()
{
    auto baseValPtr = _readValPtr();
    auto basePtrPtr = expectNonNullValOfType<FossilizedPtr<void>>(baseValPtr);

    auto targetValPtr = basePtrPtr->getTargetValPtr();
    return targetValPtr;
}


Fossil::AnyValPtr SerialReader::_readPotentiallyIndirectValPtr()
{
    auto stateType = _state.type;
    auto valPtr = _readValPtr();
    if (stateType != State::Type::Object)
    {
        auto ptrPtr = expectNonNullValOfType<FossilizedPtr<void>>(valPtr);
        valPtr = ptrPtr->getTargetValPtr();
    }
    return valPtr;
}

void SerialReader::beginTuple(Scope& scope)
{
    auto valPtr = _readValPtr();
    auto recordPtr = expectNonNullValOfType<FossilizedRecordVal>(valPtr);

    _pushRecordState(scope, recordPtr);
}

void SerialReader::beginOptional(Scope& scope)
{
    auto valPtr = _readIndirectValPtr();
    auto optionalPtr = expectPossiblyNullValOfType<FossilizedOptionalObjBase>(valPtr);
    bool hasValue = optionalPtr->hasValue();

    _pushState(scope);

    _state.type = State::Type::Object;
    if (hasValue)
    {
        auto heldValPtr = getAddress(optionalPtr->getValue());

        _state.remainingValueCount = 1;
        _state.dataCursor = heldValPtr.getDataPtr();
        _state.layoutCursor = heldValPtr.getLayout();
    }
}

void SerialReader::handleString(String& value)
{
    auto valPtr = _readPotentiallyIndirectValPtr();
    auto stringPtr = expectPossiblyNullValOfType<FossilizedStringObj>(valPtr);
    if (!stringPtr)
    {
        value = String();
    }
    else
    {
        value = stringPtr->get();
    }
}

void SerialReader::_pushContainerState(
    Scope& scope,
    Fossil::ValPtr<FossilizedContainerObjBase> containerObjPtr)
{
    // The elements of a container object start immediately
    // at its in-memory address, so we can use the pointer
    // to the container object as the pointer to its data.
    //
    auto containerDataPtr = containerObjPtr.getDataPtr();
    auto containerLayout = containerObjPtr.getLayout();

    auto elementCount = (uint32_t)containerObjPtr->getElementCount();

    FossilizedValLayout const* elementLayout = containerLayout->elementLayout;
    auto elementStride = containerLayout->elementStride;

    _pushState(scope);

    _state.type = State::Type::Container;
    _state.dataCursor = containerDataPtr;
    _state.layoutCursor = elementLayout;
    _state.dataStride = elementStride;
    _state.remainingValueCount = elementCount;
}

void SerialReader::_pushRecordState(Scope& scope, Fossil::ValPtr<FossilizedRecordVal> recordPtr)
{
    auto recordDataPtr = recordPtr.getDataPtr();
    auto recordLayout = recordPtr.getLayout();

    auto fieldCount = recordLayout->fieldCount;

    _pushState(scope);
    _state.type = State::Type::Record;
    _state.dataCursor = recordDataPtr;
    _state.remainingValueCount = fieldCount;
    if (fieldCount != 0)
    {
        auto fieldInfo = recordLayout->getField(0);
        _state.layoutCursor = fieldInfo->layout;
        _state.fieldCursor = fieldInfo;
    }
}


void SerialReader::beginArray(Scope& scope)
{
    auto valPtr = _readPotentiallyIndirectValPtr();
    auto arrayPtr = expectPossiblyNullValOfType<FossilizedArrayObjBase>(valPtr);

    _pushContainerState(scope, arrayPtr);
}

void SerialReader::beginDictionary(Scope& scope)
{
    auto valPtr = _readPotentiallyIndirectValPtr();
    auto dictionaryPtr = expectPossiblyNullValOfType<FossilizedDictionaryObjBase>(valPtr);

    _pushContainerState(scope, dictionaryPtr);
}

void SerialReader::beginStruct(Scope& scope)
{
    auto valPtr = _readValPtr();
    auto recordPtr = expectNonNullValOfType<FossilizedRecordVal>(valPtr);

    _pushRecordState(scope, recordPtr);
}

} // namespace Fossil
} // namespace Slang
