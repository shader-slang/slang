// slang-serialize-fossil.cpp
#include "slang-serialize-fossil.h"

#include "../core/slang-blob.h"

namespace Slang
{
namespace Fossil
{

//
// SerialWriter
//

SerialWriter::SerialWriter(ChunkBuilder* chunk)
{
    _initialize(chunk);
}

SerialWriter::SerialWriter(SlabBuilder& slab)
{
    auto chunk = slab.addChunk();
    _initialize(chunk);
}

void SerialWriter::_initialize(ChunkBuilder* chunk)
{
    _slab = chunk->getParentSlab();

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
    headerChunk->writeData(&header.totalSizeIncludingHeader, sizeof(header.totalSizeIncludingHeader));
    headerChunk->writeData(&header.flags, sizeof(header.flags));

    // The main reason we are writing the fields manually is
    // that the last field of the header is a relative pointer
    // to the root-value chunk.
    //
    headerChunk->writeRelativePtr<Fossil::RelativePtrOffset>(rootValueChunk);

    // The root value should always be a variant, and we want to
    // set up to write into it in a reasonable way.
    //
    auto rootPtrLayout = new PtrLayoutObj(nullptr);
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
    if (_shouldEmitWithPointerIndirection(FossilizedValKind::String))
    {
        if (size == 0)
        {
            _writeNull();
            return;
        }

        if (auto found = _mapStringToChunk.tryGetValue(value))
        {
            auto existingChunk = *found;

            auto ptrLayout = (ContainerLayoutObj*) _reserveDestinationForWrite(FossilizedValKind::Ptr);
            LayoutObj::_merge(ptrLayout->baseLayout, FossilizedValKind::String);

            _commitWrite(ValInfo::relativePtrTo(existingChunk));
            return;
        }
    }

    _pushPotentiallyIndirectValueScope(FossilizedValKind::String);

    auto data = value.getBuffer();
    _writeValueRaw(ValInfo::rawData(data, size+1, 1));

    auto chunk = _popPotentiallyIndirectValueScope();

    auto rawSize = UInt32(size);
    chunk->addPrefixData(&rawSize, sizeof(rawSize));

    _mapStringToChunk.addIfNotExists(value, chunk);
}

void SerialWriter::beginArray()
{
    _pushContainerScope(FossilizedValKind::Array);
}

void SerialWriter::endArray()
{
    _popContainerScope();
}

void SerialWriter::beginDictionary()
{
    _pushContainerScope(FossilizedValKind::Dictionary);
}

void SerialWriter::endDictionary()
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

void SerialWriter::beginStruct()
{
    _pushInlineValueScope(FossilizedValKind::Struct);
}

void SerialWriter::endStruct()
{
    _popInlineValueScope();
}

void SerialWriter::beginVariant()
{
    _pushVariantScope();
    _pushInlineValueScope(FossilizedValKind::Struct);
}

void SerialWriter::endVariant()
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

void SerialWriter::beginTuple()
{
    _pushInlineValueScope(FossilizedValKind::Tuple);
}

void SerialWriter::endTuple()
{
    _popInlineValueScope();
}

void SerialWriter::beginOptional()
{
    _pushIndirectValueScope(FossilizedValKind::Optional);
}

void SerialWriter::endOptional()
{
    _popIndirectValueScope();
}

void SerialWriter::handleSharedPtr(void*& value, Callback callback, void* userData)
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
    auto chunk = _slab->addChunk();

    auto fossilizedObject = new FossilizedObjectInfo();
    fossilizedObject->chunk = chunk;
    fossilizedObject->ptrLayout = ptrLayout;
    fossilizedObject->liveObjectPtr = liveObjectPtr;
    fossilizedObject->callback = callback;
    fossilizedObject->userData = userData;

    _fossilizedObjects.add(fossilizedObject);
    _mapLiveObjectPtrToFossilizedObject.add(liveObjectPtr, fossilizedObject);

    _commitWrite(ValInfo::relativePtrTo(chunk));
}

void SerialWriter::handleUniquePtr(void*& value, Callback callback, void* userData)
{
    // We treat all pointers as shared pointers, because there isn't really
    // an optimized representation we would want to use for the unique case.
    //
    handleSharedPtr(value, callback, userData);
}

void SerialWriter::handleDeferredObjectContents(void* valuePtr, Callback callback, void* userData)
{
    // Because we are already deferring writing of the *entirety* of
    // an object's members as part of how `handleSharedPtr()` works,
    // we don't need to implement deferral at this juncture.
    //
    // (In practice the `handleDeferredObjectContents()` operation is
    // more for the benefit of reading than writing).
    //
    callback(valuePtr, userData);
}

SerialWriter::LayoutObj* SerialWriter::LayoutObj::_create(FossilizedValKind kind)
{
    switch (kind)
    {
    case FossilizedValKind::Array:
    case FossilizedValKind::Optional:
    case FossilizedValKind::Dictionary:
        return new ContainerLayoutObj(kind, nullptr);

    case FossilizedValKind::Ptr:
        return new ContainerLayoutObj(kind, nullptr, sizeof(Fossil::RelativePtrOffset), sizeof(Fossil::RelativePtrOffset));

    case FossilizedValKind::Variant:
        // A variant is being treated like a container at this point,
        // because it wants to be able to track the layout of what it
        // ended up holding...
        //
        return new ContainerLayoutObj(kind, nullptr);

    case FossilizedValKind::Bool:
    case FossilizedValKind::Int8:
    case FossilizedValKind::UInt8:
        return new SimpleLayoutObj(kind, 1);

    case FossilizedValKind::Int32:
    case FossilizedValKind::UInt32:
    case FossilizedValKind::Float32:
        return new SimpleLayoutObj(kind, 4);

    case FossilizedValKind::Int64:
    case FossilizedValKind::UInt64:
    case FossilizedValKind::Float64:
        return new SimpleLayoutObj(kind, 8);

    case FossilizedValKind::String:
        return new SimpleLayoutObj(kind);

    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
        return new RecordLayoutObj(kind);

    default:
        SLANG_UNEXPECTED("implement me!");
        UNREACHABLE_RETURN(nullptr);
    }
}

SerialWriter::LayoutObj* SerialWriter::LayoutObj::_merge(LayoutObj*& dst, FossilizedValKind kind)
{
    if (!dst)
    {
        dst = _create(kind);
    }

    if (dst->kind != kind)
    {
        SLANG_UNEXPECTED("type mismatch during serialization");
    }

    // As a special case, if the right-hand-side is a variant,
    // then we want to have a unique layout object for each
    // instance.
    //
    if (kind == FossilizedValKind::Variant)
    {
        auto src = _create(kind);
        return src;
    }

    return dst;
}

void SerialWriter::LayoutObj::_merge(LayoutObj*& dst, LayoutObj* src)
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

    _merge(dst, src->getKind());

    switch (src->getKind())
    {
    case FossilizedValKind::Array:
    case FossilizedValKind::Optional:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::Ptr:
    {
        auto dstContainer = (ContainerLayoutObj*)dst;
        auto srcContainer = (ContainerLayoutObj*)src;
        _merge(dstContainer->baseLayout, srcContainer->baseLayout);
    }
    break;

    case FossilizedValKind::String:
        break;

    case FossilizedValKind::Variant:
        break;

    default:
        SLANG_UNEXPECTED("implement me!");
        break;
    }
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
        return sizeof(Fossil::RelativePtrOffset);

    case Kind::ContentsOfChunk:
        return chunk->getAlignment();

    case Kind::RawData:
        return data.alignment;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

#if 0
SerialWriter::LayoutObj* SerialWriter::_createUnknownLayout()
{
    auto layoutObj = new ContainerLayoutObj(FossilizedValKind(-1), nullptr);
    return layoutObj;
}

SerialWriter::LayoutObj* SerialWriter::_getContainerLayout(FossilizedValKind kind, LayoutObj* elementLayout)
{
    // TODO: caching!
    auto layoutObj = new ContainerLayoutObj(FossilizedValKind::Ptr, elementLayout);
    return layoutObj;
}

SerialWriter::LayoutObj* SerialWriter::_getPtrLayout(LayoutObj* valLayout)
{
    // TODO: caching!

    auto ptrSize = sizeof(Fossil::RelativePtrOffset);
    auto layoutObj = new ContainerLayoutObj(FossilizedValKind::Ptr, valLayout, ptrSize, ptrSize);
    return layoutObj;
}

SerialWriter::LayoutObj* SerialWriter::_getLayout(FossilizedValKind kind, Size size, Size alignment)
{
    // TODO: caching!
    // TODO: validate that the size/alignment makes sense for the kind...

    auto layoutObj = new LayoutObj(kind, size, alignment);
    return layoutObj;
}
#endif

#define ENABLE_DUMP 0

#if ENABLE_DUMP

static int gDebugIndent = 0;

static void _dumpIndent()
{
    for (int i = 0; i < gDebugIndent; ++i)
        fprintf(stderr, "  ");
}
#endif

void SerialWriter::_pushInlineValueScope(FossilizedValKind kind)
{
#if ENABLE_DUMP
    _dumpIndent();
    fprintf(stderr, "beginInlineValue %d\n", int(kind));
    gDebugIndent++;
#endif

    auto layout = _reserveDestinationForWrite(kind);
    _pushState(layout);
}

void SerialWriter::_popInlineValueScope()
{
#if ENABLE_DUMP
    gDebugIndent--;
    _dumpIndent();
    fprintf(stderr, "endInlineValue\n");
#endif

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
    _pushPotentiallyIndirectValueScope(FossilizedValKind::Variant);
}

void SerialWriter::_popVariantScope()
{
    SLANG_ASSERT(_state.layout);
    SLANG_ASSERT(_state.layout->kind == FossilizedValKind::Variant);
    auto variantLayout = (ContainerLayoutObj*) _state.layout;
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
#if ENABLE_DUMP
    _dumpIndent();
    fprintf(stderr, "_beginIndirectValue %d\n", int(kind));
    gDebugIndent++;
#endif

    if (_shouldEmitWithPointerIndirection(kind))
    {
        _pushIndirectValueScope(kind);
    }
    else
    {
        auto valueLayout = _reserveDestinationForWrite(kind);
        _pushState(valueLayout);
    }
}

ChunkBuilder* SerialWriter::_popPotentiallyIndirectValueScope()
{
#if ENABLE_DUMP
    gDebugIndent--;
    _dumpIndent();
    fprintf(stderr, "_endContainer\n");
#endif

    auto valueLayout = _state.layout;
    auto valueChunk = _state.chunk;
    _popState();

    auto valueKind = valueLayout->getKind();
    if (_shouldEmitWithPointerIndirection(valueKind))
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
#if ENABLE_DUMP
    _dumpIndent();
    fprintf(stderr, "_beginKnownIndirectValue %d\n", int(kind));
    gDebugIndent++;
#endif

    auto ptrLayout = (PtrLayoutObj*)_reserveDestinationForWrite(FossilizedValKind::Ptr);
    auto valueLayout = LayoutObj::_merge(ptrLayout->baseLayout, kind);

    _pushState(valueLayout);
}

void SerialWriter::_popIndirectValueScope()
{
#if ENABLE_DUMP
    gDebugIndent--;
    _dumpIndent();
    fprintf(stderr, "_endContainer\n");
#endif

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

    _slab->addChunk(valueChunk);

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

    _state.chunk = _slab->createUnparentedChunk();
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
        _state.chunk->writeRelativePtr<Fossil::RelativePtrOffset>(val.chunk);
        break;

    case ValInfo::Kind::ContentsOfChunk:
        {
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

#if 0
void SerialWriter::_writeValueRaw(LayoutObj* layout, ValInfo const& val)
{
    SLANG_ASSERT(layout);
    if (_state.chunk)
    {
        _state.chunk->writePaddingToAlignTo(layout->getAlignment());
    }
    _writeValueRaw(val);
    if (_state.chunk)
    {
        _state.chunk->setAlignmentToAtLeast(layout->getAlignment());
    }
}
#endif

bool SerialWriter::_shouldEmitWithPointerIndirection(FossilizedValKind kind)
{
    switch (kind)
    {
    default:
        return false;

    case FossilizedValKind::Optional:
        return true;

    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::String:
    case FossilizedValKind::Variant:
        break;
    }

    switch (_state.layout->getKind())
    {
    default:
        return true;

    case FossilizedValKind::Optional:
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
        auto tupleLayout = (TupleLayoutObj*)_state.layout;
        auto elementIndex = _state.elementCount;
        auto& elementLayout = tupleLayout->_getField(elementIndex).layout;
        return elementLayout;
    }
    break;

    case FossilizedValKind::Ptr:
    case FossilizedValKind::Optional:
    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::Variant:
    {
        auto containerLayout = (ContainerLayoutObj*)_state.layout;
        auto& elementLayout = containerLayout->baseLayout;
        return elementLayout;
    }
    break;

    default:
        SLANG_UNEXPECTED("implement me!");
        break;
    }
}

SerialWriter::LayoutObj* SerialWriter::_reserveDestinationForWrite(FossilizedValKind srcKind)
{
    return LayoutObj::_merge(_reserveDestinationForWrite(), srcKind);
}

SerialWriter::LayoutObj* SerialWriter::_reserveDestinationForWrite(LayoutObj* srcLayout)
{
    SLANG_ASSERT(srcLayout != nullptr);
    LayoutObj::_merge(_reserveDestinationForWrite(), srcLayout);
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
        auto tupleLayout = (TupleLayoutObj*)_state.layout;
        auto elementIndex = _state.elementCount++;
        auto& fieldInfo = tupleLayout->_getField(elementIndex);

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

    case FossilizedValKind::Optional:
    case FossilizedValKind::Ptr:
    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::Variant:
    {
        auto elementIndex = _state.elementCount++;

        switch (outerKind)
        {
        case FossilizedValKind::Optional:
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

    #if 0
    case FossilizedValKind::Variant:
        {
            auto elementIndex = _state.elementCount++;
            if (elementIndex > 0)
            {
                SLANG_UNEXPECTED(
                    "error during serialization: variant with more than one value inside!!");
            }

            _writeValueRaw(layout, val);
        }
        break;
        #endif

    default:
        SLANG_UNEXPECTED("implement me!");
        break;
    }
}

void SerialWriter::_writeSimpleValue(FossilizedValKind kind, void const* data, size_t size, size_t alignment)
{
    auto layout = _reserveDestinationForWrite(kind);
    SLANG_ASSERT(layout->size == size);
    SLANG_ASSERT(layout->alignment = alignment);
    _commitWrite(ValInfo::rawData(data, size, alignment));
}

#if 0
bool SerialWriter::_trySkipValue()
{
    // TODO: determine if we *can* skip a value in this context.
    //
    // If we can skip a value, then we write a zero-byte
    // null instead.
    //
    _writeNull();
    return true;
}
#endif

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

        fossilizedObject->callback(&fossilizedObject->liveObjectPtr, fossilizedObject->userData);
    }

    // Once we've written out all the payload data, we can start to work on
    // serializing layout information for all the variant values that were
    // written.
    //
    for (auto variantInfo : _variants)
    {
        auto layoutChunk = _getOrCreateChunkForLayout(variantInfo.layout);
        variantInfo.chunk->addPrefixRelativePtr<Fossil::RelativePtrOffset>(layoutChunk);
    }
}

ChunkBuilder* SerialWriter::_getOrCreateChunkForLayout(LayoutObj* layout)
{
    if (!layout)
        return nullptr;

    if (auto existingChunk = layout->chunk)
        return existingChunk;

    LayoutObjKey key = {layout};
    if (auto found = _mapLayoutObjToChunk.tryGetValue(key))
    {
        auto existingChunk = *found;
        layout->chunk = existingChunk;
        return existingChunk;
    }

    // TODO: next we should try to get a cached result
    // (so that we aren't re-creating everything from
    // scratch every time...).
    //


    // If no existing layout has been created, then we'll create one.
    //
    auto chunk = _slab->addChunk();
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
    case FossilizedValKind::Optional:
    {
        auto containerLayout = (ContainerLayoutObj*)layout;
        auto elementLayout = containerLayout->baseLayout;
        auto elementLayoutChunk = _getOrCreateChunkForLayout(elementLayout);
        chunk->writeRelativePtr<Fossil::RelativePtrOffset>(elementLayoutChunk);
    }
    break;

    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    {
        auto containerLayout = (ContainerLayoutObj*)layout;
        auto elementLayout = containerLayout->baseLayout;
        auto elementLayoutChunk = _getOrCreateChunkForLayout(elementLayout);
        chunk->writeRelativePtr<Fossil::RelativePtrOffset>(elementLayoutChunk);

        UInt32 elementStride = 0;
        if (elementLayout)
        {
            elementStride = UInt32(roundUpToAlignment(elementLayout->size, elementLayout->alignment));
            SLANG_ASSERT(elementStride != 0);
        }
        chunk->writeData(&elementStride, sizeof(elementStride));
    }
    break;

    case FossilizedValKind::Struct:
    case FossilizedValKind::Tuple:
    {
        auto recordLayout = (RecordLayoutObj*)layout;

        auto fieldCount = UInt32(recordLayout->fields.getCount());
        chunk->writeData(&fieldCount, sizeof(fieldCount));

        bool first = true;
        for (auto field : recordLayout->fields)
        {
            auto fieldLayoutChunk = _getOrCreateChunkForLayout(field.layout);
            chunk->writeRelativePtr<Fossil::RelativePtrOffset>(fieldLayoutChunk);

            auto fieldOffset = UInt32(field.offset);
            chunk->writeData(&fieldOffset, sizeof(fieldOffset));

            if (!first)
            {
                SLANG_ASSERT(fieldOffset != 0);
            }
            first = false;
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

    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::Optional:
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

        if (thisRecord->fields.getCount() != thatRecord->fields.getCount())
            return false;

        auto fieldCount = thisRecord->fields.getCount();
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

    case FossilizedValKind::Array:
    case FossilizedValKind::Dictionary:
    case FossilizedValKind::Optional:
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

        hasher.hashValue(record->fields.getCount());

        for (auto field : record->fields)
        {
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

SerialReader::SerialReader(FossilizedValRef valRef)
{
    _state.type = State::Type::Root;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = 1;
}

SerialReader::~SerialReader()
{
    _flush();
}

SerializationMode SerialReader::getMode()
{
    return SerializationMode::Read;
}

void SerialReader::handleBool(bool& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedBoolVal>(valRef)->getValue();
}

void SerialReader::handleInt8(int8_t& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedInt8Val>(valRef)->getValue();
}

void SerialReader::handleInt16(int16_t& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedInt16Val>(valRef)->getValue();
}

void SerialReader::handleInt32(Int32& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedInt32Val>(valRef)->getValue();
}

void SerialReader::handleInt64(Int64& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedInt64Val>(valRef)->getValue();
}

void SerialReader::handleUInt8(uint8_t& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedUInt8Val>(valRef)->getValue();
}

void SerialReader::handleUInt16(uint16_t& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedUInt16Val>(valRef)->getValue();
}

void SerialReader::handleUInt32(UInt32& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedUInt32Val>(valRef)->getValue();
}

void SerialReader::handleUInt64(UInt64& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedUInt64Val>(valRef)->getValue();
}

void SerialReader::handleFloat32(float& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedFloat32Val>(valRef)->getValue();
}

void SerialReader::handleFloat64(double& value)
{
    auto valRef = _readValRef();
    value = as<FossilizedFloat64Val>(valRef)->getValue();
}

void SerialReader::handleString(String& value)
{
    auto valRef = _readPotentiallyIndirectValRef();
    if (!valRef)
    {
        value = String();
    }
    else
    {
        value = as<FossilizedStringObj>(valRef)->getValue();
    }
}

void SerialReader::beginArray()
{
    auto valRef = _readPotentiallyIndirectValRef();
    auto arrayRef = as<FossilizedContainerObj>(valRef);

    _pushState();

    _state.type = State::Type::Array;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = getElementCount(arrayRef);
}

void SerialReader::endArray()
{
    _popState();
}

void SerialReader::beginDictionary()
{
    auto valRef = _readPotentiallyIndirectValRef();
    auto dictionaryRef = as<FossilizedContainerObj>(valRef);

    _pushState();

    _state.type = State::Type::Dictionary;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = getElementCount(dictionaryRef);
}

void SerialReader::endDictionary()
{
    _popState();
}

bool SerialReader::hasElements()
{
    return _state.elementIndex < _state.elementCount;
}

void SerialReader::beginStruct()
{
    auto valRef = _readValRef();
    auto recordRef = as<FossilizedRecordVal>(valRef);

    _pushState();

    _state.type = State::Type::Struct;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = getFieldCount(recordRef);
}

void SerialReader::endStruct()
{
    _popState();
}

void SerialReader::beginVariant()
{
    auto valRef = _readPotentiallyIndirectValRef();
    auto variantRef = as<FossilizedVariantObj>(valRef);

    auto contentValRef = getVariantContent(variantRef);
    auto contentRecordRef = as<FossilizedRecordVal>(contentValRef);

    _pushState();

    _state.type = State::Type::Struct;
    _state.baseValue = contentValRef;
    _state.elementIndex = 0;
    _state.elementCount = getFieldCount(contentRecordRef);
}

void SerialReader::endVariant()
{
    _popState();
}

void SerialReader::handleFieldKey(char const* name, Int index)
{
    // For now we are ignoring field keys, and treating
    // structs as basically equivalent to tuples.
    SLANG_UNUSED(name);
    SLANG_UNUSED(index);
}

void SerialReader::beginTuple()
{
    auto valRef = _readValRef();
    auto recordRef = as<FossilizedRecordVal>(valRef);

    _pushState();

    _state.type = State::Type::Tuple;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = getFieldCount(recordRef);
}

void SerialReader::endTuple()
{
    _popState();
}

void SerialReader::beginOptional()
{
    auto valRef = _readIndirectValRef();
    auto optionalRef = as<FossilizedOptionalObj>(valRef);

    _pushState();

    _state.type = State::Type::Optional;
    _state.baseValue = valRef;
    _state.elementIndex = 0;
    _state.elementCount = Count(hasValue(optionalRef));
}

void SerialReader::endOptional()
{
    _popState();
}

void SerialReader::handleSharedPtr(void*& value, Callback callback, void* userData)
{
    // The fossilized value at our cursor must be a pointer,
    // and we can resolve what it is pointing to easily enough.
    //
    auto valRef = _readValRef();
    auto ptrRef = as<FossilizedPtrVal>(valRef);
    auto targetValRef = getPtrTarget(ptrRef);

    // The logic here largely mirrors what appears in
    // `SerialWriter::handleSharedPtr`.
    //
    // We first check for an explicitly written null pointer.
    // If we find one our work is very easy.
    //
    if (!targetValRef)
    {
        value = nullptr;
        return;
    }

    // Now we need to check if we've previously read in
    // a reference to the same object.
    //
    if (auto found = _mapFossilizedObjectPtrToObjectInfo.tryGetValue(targetValRef.getData()))
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
    auto objectInfo = new ObjectInfo();
    _mapFossilizedObjectPtrToObjectInfo.add(targetValRef.getData(), objectInfo);
    objectInfo->fossilizedObjectRef = targetValRef;

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
    _pushState();
    _state.type = State::Type::Object;
    _state.baseValue = objectInfo->fossilizedObjectRef;
    _state.elementIndex = 0;
    _state.elementCount = 1;

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
    callback(&objectInfo->resurrectedObjectPtr, userData);

    _popState();

    objectInfo->state = ObjectState::ReadingComplete;

    value = objectInfo->resurrectedObjectPtr;
}

void SerialReader::handleUniquePtr(void*& value, Callback callback, void* userData)
{
    // We treat all pointers as shared pointers, because there isn't really
    // an optimized representation we would want to use for the unique case.
    //
    handleSharedPtr(value, callback, userData);
}

void SerialReader::handleDeferredObjectContents(
    void* valuePtr,
    Callback callback,
    void* userData)
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
    deferredAction.userData = userData;

    _deferredActions.add(deferredAction);
}

void SerialReader::_flush()
{
    // We need to flush any actions that were deferred
    // and are still pending.
    //
    while (_deferredActions.getCount() != 0)
    {
        // TODO: For simplicity we are using the `_deferredActions`
        // array as a stack (LIFO), but it would be good to
        // check whether there is a menaingful difference in how
        // large the array would need to grow for a FIFO vs. LIFO,
        // and pick the better option.
        //
        auto deferredAction = _deferredActions.getLast();
        _deferredActions.removeLast();

        _state = deferredAction.savedState;
        deferredAction.callback(deferredAction.resurrectedObjectPtr, deferredAction.userData);
    }
}

FossilizedValRef SerialReader::_readValRef()
{
    switch (_state.type)
    {
    case State::Type::Root:
    case State::Type::Object:
        SLANG_ASSERT(_state.elementCount == 1);
        SLANG_ASSERT(_state.elementIndex == 0);
        _state.elementIndex++;
        return _state.baseValue;

    case State::Type::Struct:
    case State::Type::Tuple:
    {
        SLANG_ASSERT(_state.elementIndex < _state.elementCount);
        auto index = _state.elementIndex++;

        auto recordRef = as<FossilizedRecordVal>(_state.baseValue);
        return getField(recordRef, index);
    }

    case State::Type::Optional:
    {
        SLANG_ASSERT(_state.elementCount == 1);
        SLANG_ASSERT(_state.elementIndex == 0);

        auto optionalRef = as<FossilizedOptionalObj>(_state.baseValue);
        return getValue(optionalRef);
    }

    case State::Type::Array:
    case State::Type::Dictionary:
    {
        SLANG_ASSERT(_state.elementIndex < _state.elementCount);
        auto index = _state.elementIndex++;

        auto containerRef = as<FossilizedContainerObj>(_state.baseValue);
        return getElement(containerRef, index);
    }

    default:
        SLANG_UNEXPECTED("implement me!");
        break;
    }
}

FossilizedValRef SerialReader::_readIndirectValRef()
{
    auto ptrValRef = _readValRef();
    auto ptrRef = as<FossilizedPtrVal>(ptrValRef);

    auto valRef = getPtrTarget(ptrRef);
    return valRef;
}


FossilizedValRef SerialReader::_readPotentiallyIndirectValRef()
{
    auto valRef = _readValRef();
    if (auto ptrRef = as<FossilizedPtrVal>(valRef))
    {
        return getPtrTarget(ptrRef);
    }
    return valRef;
}

void SerialReader::_pushState()
{
    _stack.add(_state);
}

void SerialReader::_popState()
{
    SLANG_ASSERT(_stack.getCount() != 0);
    _state = _stack.getLast();
    _stack.removeLast();
}

} // namespace Fossil
} // namespace Slang
