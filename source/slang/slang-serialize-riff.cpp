// slang-serialize-riff.cpp
#include "slang-serialize-riff.h"

namespace Slang
{

//
// RIFFSerialWriter
//

RIFFSerialWriter::RIFFSerialWriter(RIFF::ChunkBuilder* chunk, FourCC type)
    : _cursor(chunk)
{
    _initialize(type);
}

RIFFSerialWriter::RIFFSerialWriter(RIFF::Builder& riff, FourCC type)
    : _cursor(riff)
{
    _initialize(type);
}

RIFFSerialWriter::~RIFFSerialWriter()
{
    // We need to flush any pending operations to
    // write objects into the object definition list chunk.
    //
    _flush();
}

SerializationMode RIFFSerialWriter::getMode()
{
    return SerializationMode::Write;
}

void RIFFSerialWriter::handleBool(bool& value)
{
    _cursor.addDataChunk(value ? RIFFSerial::kTrueFourCC : RIFFSerial::kFalseFourCC, nullptr, 0);
}

void RIFFSerialWriter::handleInt8(int8_t& value)
{
    _writeInt(value);
}

void RIFFSerialWriter::handleInt16(int16_t& value)
{
    _writeInt(value);
}

void RIFFSerialWriter::handleInt32(Int32& value)
{
    _writeInt(value);
}

void RIFFSerialWriter::handleInt64(Int64& value)
{
    _writeInt(value);
}

void RIFFSerialWriter::handleUInt8(uint8_t& value)
{
    _writeUInt(value);
}

void RIFFSerialWriter::handleUInt16(uint16_t& value)
{
    _writeUInt(value);
}

void RIFFSerialWriter::handleUInt32(UInt32& value)
{
    _writeUInt(value);
}

void RIFFSerialWriter::handleUInt64(UInt64& value)
{
    _writeUInt(value);
}

void RIFFSerialWriter::handleFloat32(float& value)
{
    _writeFloat(value);
}

void RIFFSerialWriter::handleFloat64(double& value)
{
    _writeFloat(value);
}

void RIFFSerialWriter::handleString(String& value)
{
    _cursor.addDataChunk(RIFFSerial::kStringFourCC, value.getBuffer(), value.getLength());
}

void RIFFSerialWriter::_writeInt(Int64 value)
{
    // We pick a 32-bit representation if it can
    // faithfully represent the value, and a 64-bit
    // representation otherwise.
    //
    if (Int32(value) == value)
    {
        auto v = Int32(value);
        _cursor.addDataChunk(RIFFSerial::kInt32FourCC, &v, sizeof(v));
    }
    else
    {
        _cursor.addDataChunk(RIFFSerial::kInt64FourCC, &value, sizeof(value));
    }
}

void RIFFSerialWriter::_writeUInt(UInt64 value)
{
    // We pick a 32-bit representation if it can
    // faithfully represent the value, and a 64-bit
    // representation otherwise.
    //
    if (UInt32(value) == value)
    {
        auto v = UInt32(value);
        _cursor.addDataChunk(RIFFSerial::kUInt32FourCC, &v, sizeof(v));
    }
    else
    {
        _cursor.addDataChunk(RIFFSerial::kUInt64FourCC, &value, sizeof(value));
    }
}

void RIFFSerialWriter::_writeFloat(double value)
{
    // We pick a 32-bit representation if it can
    // faithfully represent the value, and a 64-bit
    // representation otherwise.
    //
    if (float(value) == value)
    {
        auto v = float(value);
        _cursor.addDataChunk(RIFFSerial::kFloat32FourCC, &v, sizeof(v));
    }
    else
    {
        _cursor.addDataChunk(RIFFSerial::kFloat64FourCC, &value, sizeof(value));
    }
}

void RIFFSerialWriter::beginArray()
{
    _cursor.beginListChunk(RIFFSerial::kArrayFourCC);
}

void RIFFSerialWriter::endArray()
{
    _cursor.endChunk();
}

void RIFFSerialWriter::beginDictionary()
{
    _cursor.beginListChunk(RIFFSerial::kDictionaryFourCC);
}

void RIFFSerialWriter::endDictionary()
{
    _cursor.endChunk();
}

bool RIFFSerialWriter::hasElements()
{
    return false;
}

void RIFFSerialWriter::beginStruct()
{
    _cursor.beginListChunk(RIFFSerial::kStructFourCC);
}

void RIFFSerialWriter::endStruct()
{
    _cursor.endChunk();
}

void RIFFSerialWriter::beginVariant()
{
    beginStruct();
}

void RIFFSerialWriter::endVariant()
{
    endStruct();
}

void RIFFSerialWriter::handleFieldKey(char const* name, Int index)
{
    // For now we are ignoring field keys, and treating
    // structs as basically equivalent to tuples.
    SLANG_UNUSED(name);
    SLANG_UNUSED(index);
}

void RIFFSerialWriter::beginTuple()
{
    _cursor.beginListChunk(RIFFSerial::kTupleFourCC);
}

void RIFFSerialWriter::endTuple()
{
    _cursor.endChunk();
}

void RIFFSerialWriter::beginOptional()
{
    _cursor.beginListChunk(RIFFSerial::kOptionalFourCC);
}

void RIFFSerialWriter::endOptional()
{
    _cursor.endChunk();
}

void RIFFSerialWriter::handleSharedPtr(void*& value, Callback callback, void* userData)
{
    // Because we are writing, we only care about the
    // pointer that is already present in `value`.
    //
    void* ptr = value;

    // The first special case we check for is a null pointer,
    // which we can serialize as an inline value.
    //
    if (ptr == nullptr)
    {
        _cursor.addDataChunk(RIFFSerial::kNullFourCC, nullptr, 0);
        return;
    }

    // Next, we check to see if we have encountered this
    // pointer before, in which case we've already allocated
    // an index for it in the object definition list, and
    // we can simply write a reference to that index.
    //
    if (auto found = _mapPtrToObjectIndex.tryGetValue(ptr))
    {
        auto objectIndex = *found;
        _writeObjectReference(objectIndex);
        return;
    }

    // If we have a non-null pointer that we haven't seen
    // before, then we will allocate a new entry in the
    // object definition list, and the pointer itself
    // will be written as a reference to that entry.
    //
    auto objectIndex = ObjectIndex(_objects.getCount());
    _mapPtrToObjectIndex.add(ptr, objectIndex);
    _writeObjectReference(objectIndex);

    // At this point we've correctly written the *reference*
    // to the object (and will be able to write further
    // references later if we see an identical pointer),
    // but we also need to make sure that the *definition*
    // of the object gets written into the object definition
    // list chunk.
    //
    // The `callback` that was passed in can be used to
    // write out the members of the object, but if we
    // simply invoked it here and now we would be at risk
    // of introducing unbounded recursion in cases where
    // the object graph contains very long pointer chains.
    //
    // (Note that we are not at risk of *infinite* recursion,
    // because we have already cached the index for the
    // object into `_mapPtrToObjectIndex`)
    //
    // We will simply add an entry to our `_objects` array
    // to represent the to-be-written object, and store
    // the pointer and callback there so that we can write
    // everything out later, in `_flush()`.
    //
    ObjectInfo objectInfo;
    objectInfo.ptr = ptr;
    objectInfo.callback = callback;
    objectInfo.userData = userData;
    _objects.add(objectInfo);
}

void RIFFSerialWriter::handleUniquePtr(void*& value, Callback callback, void* userData)
{
    // We treat all pointers as shared pointers, because there isn't really
    // an optimized representation we would want to use for the unique case.
    //
    handleSharedPtr(value, callback, userData);
}

void RIFFSerialWriter::handleDeferredObjectContents(
    void* valuePtr,
    Callback callback,
    void* userData)
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

void RIFFSerialWriter::_writeObjectReference(ObjectIndex index)
{
    _cursor.addDataChunk(RIFFSerial::kObjectReferenceFourCC, &index, sizeof(index));
}

void RIFFSerialWriter::_initialize(FourCC type)
{
    // The entire content that we write will be nested
    // in a single list chunk, with the type that was
    // passed in.
    //
    _cursor.beginListChunk(type);

    // The first child chunk needs to be the object
    // definition list chunk, so we create it up front.
    //
    _objectDefinitionListChunk = _cursor.addListChunk(RIFFSerial::kObjectDefinitionListFourCC);
}

void RIFFSerialWriter::_flush()
{
    // At this point we might have zero or more object
    // waiting to be written into the object definition list
    // chunk, and we need to make sure that they all
    // get a chance to write their content out.
    //
    _cursor.setCurrentChunk(_objectDefinitionListChunk);

    // Note that we do *not* compute `_objects.getCount()` outside
    // of the loop here, because writing out one object definition
    // could cause other objects to be referenced, which could
    // in turn add more entries to `_objects` that need to be
    // written out.
    //
    while (_writtenObjectDefinitionCount < _objects.getCount())
    {
        auto objectIndex = _writtenObjectDefinitionCount++;
        auto objectInfo = _objects[objectIndex];

        // We shouldn't ever be putting a null pointer into the
        // object definition list; there is logic in `handleSharedPtr()`
        // that explicitly checks for a null pointer and does an
        // early-exit in that case.
        //
        SLANG_ASSERT(objectInfo.ptr);

        // The callback that was passed into `handleSharedPtr()` should
        // be able to write out the value of the pointed-to object.
        //
        // Note that we are passing the *address* of `objectInfo.ptr`
        // and not just its *value*, because this callback is used
        // for both reading and writing, and in the reading case it
        // needs to be invoked on a pointer-pointer (e.g., a `T**` when
        // serializing an object pointer `T*`) so that the callee
        // can set the pointed-to pointer to whatever object it
        // allocates or finds.
        //
        objectInfo.callback(&objectInfo.ptr, objectInfo.userData);

        // TODO(tfoley): There is an important invariant here that
        // the callback had better only write *one* value, but
        // that is not currently being enforced.
    }
}

//
// RIFFSerialReader
//

RIFFSerialReader::RIFFSerialReader(RIFF::Chunk const* chunk, FourCC type)
    : _cursor(chunk)
{
    _initialize(type);
}

RIFFSerialReader::~RIFFSerialReader()
{
    _flush();
}

SerializationMode RIFFSerialReader::getMode()
{
    return SerializationMode::Read;
}

void RIFFSerialReader::handleBool(bool& value)
{
    switch (_peekChunkType())
    {
    case RIFFSerial::kTrueFourCC:
        _advanceCursor();
        value = true;
        break;

    case RIFFSerial::kFalseFourCC:
        _advanceCursor();
        value = false;
        break;

    default:
        SLANG_UNEXPECTED("invalid format in RIFF");
        break;
    }
}

void RIFFSerialReader::handleInt8(int8_t& value)
{
    value = int8_t(_readInt());
}

void RIFFSerialReader::handleInt16(int16_t& value)
{
    value = int16_t(_readInt());
}

void RIFFSerialReader::handleInt32(Int32& value)
{
    value = Int32(_readInt());
}

void RIFFSerialReader::handleInt64(Int64& value)
{
    value = Int64(_readInt());
}

void RIFFSerialReader::handleUInt8(uint8_t& value)
{
    value = uint8_t(_readUInt());
}

void RIFFSerialReader::handleUInt16(uint16_t& value)
{
    value = uint16_t(_readUInt());
}

void RIFFSerialReader::handleUInt32(UInt32& value)
{
    value = UInt32(_readUInt());
}

void RIFFSerialReader::handleUInt64(UInt64& value)
{
    value = UInt64(_readUInt());
}

void RIFFSerialReader::handleFloat32(float& value)
{
    value = float(_readFloat());
}

void RIFFSerialReader::handleFloat64(double& value)
{
    value = double(_readFloat());
}

void RIFFSerialReader::handleString(String& value)
{
    if (_peekChunkType() != RIFFSerial::kStringFourCC)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
        return;
    }

    auto dataChunk = as<RIFF::DataChunk>(_cursor);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
        return;
    }

    auto size = dataChunk->getPayloadSize();

    value = String();
    value.appendRepeatedChar(' ', size);
    dataChunk->writePayloadInto((char*)value.getBuffer(), size);

    _advanceCursor();
}

void RIFFSerialReader::beginArray()
{
    _beginListChunk(RIFFSerial::kArrayFourCC);
}

void RIFFSerialReader::endArray()
{
    _endListChunk();
}


void RIFFSerialReader::beginDictionary()
{
    _beginListChunk(RIFFSerial::kDictionaryFourCC);
}

void RIFFSerialReader::endDictionary()
{
    _endListChunk();
}

bool RIFFSerialReader::hasElements()
{
    return _cursor.get() != nullptr;
}

void RIFFSerialReader::beginStruct()
{
    _beginListChunk(RIFFSerial::kStructFourCC);
}

void RIFFSerialReader::endStruct()
{
    _endListChunk();
}

void RIFFSerialReader::beginVariant()
{
    beginStruct();
}

void RIFFSerialReader::endVariant()
{
    endStruct();
}

void RIFFSerialReader::handleFieldKey(char const* name, Int index)
{
    // For now we are ignoring field keys, and treating
    // structs as basically equivalent to tuples.
    SLANG_UNUSED(name);
    SLANG_UNUSED(index);
}

void RIFFSerialReader::beginTuple()
{
    _beginListChunk(RIFFSerial::kTupleFourCC);
}

void RIFFSerialReader::endTuple()
{
    _endListChunk();
}

void RIFFSerialReader::beginOptional()
{
    _beginListChunk(RIFFSerial::kOptionalFourCC);
}

void RIFFSerialReader::endOptional()
{
    _endListChunk();
}

RIFFSerialReader::ObjectIndex RIFFSerialReader::_readObjectReference()
{
    if (_peekChunkType() != RIFFSerial::kObjectReferenceFourCC)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
        UNREACHABLE_RETURN(false);
    }

    auto objectIndex = _readDataChunk<ObjectIndex>();
    SLANG_ASSERT(objectIndex >= 0 && objectIndex < _objects.getCount());
    return objectIndex;
}

void RIFFSerialReader::handleSharedPtr(void*& value, Callback callback, void* userData)
{
    // The logic here largely mirrors what appears in
    // `RIFFSerialWriter::handleSharedPtr`.
    //
    // We first check for an explicitly written null pointer.
    // If we find one our work is very easy.
    //
    if (_peekChunkType() == RIFFSerial::kNullFourCC)
    {
        _advanceCursor();
        value = nullptr;
        return;
    }

    // Otherwise, we expect to find a reference to
    // an object index.
    //
    // Note that `_readObjectReference()` already asserts
    // that the index is in-bounds, so we don't repeat
    // that test here.
    //
    auto objectIndex = _readObjectReference();

    // Now we need to check if we've previously read in
    // a reference to the same object.
    //
    auto& objectInfo = _objects[objectIndex];
    if (objectInfo.state != ObjectState::Unread)
    {
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
        if (objectInfo.ptr == nullptr)
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
            if (objectInfo.state == ObjectState::ReadingInProgress)
            {
                SLANG_UNEXPECTED("circularity detected in RIFF deserialization");
            }
        }
        value = objectInfo.ptr;
        return;
    }

    // At this point we are reading a reference to an
    // object index that has not yet been read at all.
    //
    SLANG_ASSERT(objectInfo.state == ObjectState::Unread);

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
    objectInfo.state = ObjectState::ReadingInProgress;

    // We save/restore the current cursor around
    // the callback, because we need to be able
    // to return to the current state to continue
    // reading whatever comes after the pointer
    // we were invoked to read.
    //
    _pushCursor();
    _cursor = objectInfo.definitionChunk;

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
    callback(&objectInfo.ptr, userData);

    _popCursor();

    objectInfo.state = ObjectState::ReadingComplete;

    value = objectInfo.ptr;
}

void RIFFSerialReader::handleUniquePtr(void*& value, Callback callback, void* userData)
{
    // We treat all pointers as shared pointers, because there isn't really
    // an optimized representation we would want to use for the unique case.
    //
    handleSharedPtr(value, callback, userData);
}

void RIFFSerialReader::handleDeferredObjectContents(
    void* valuePtr,
    Callback callback,
    void* userData)
{
    // Unlike the case in `RIFFSerialWriter::handleDeferredObjectContents()`,
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
    deferredAction.savedCursor = _cursor;
    deferredAction.valuePtr = valuePtr;
    deferredAction.callback = callback;
    deferredAction.userData = userData;

    _deferredActions.add(deferredAction);
}

void RIFFSerialReader::_initialize(FourCC type)
{
    // All of the content will have been serialized as a single RIFF
    // list chunk (possibly a root chunk if this content comprises
    // an entire file), with the given `type`.
    //
    _beginListChunk(type);

    // The first child chunk should be the object definition list
    // chunk, and we will proactively read through all of the
    // entries in that chunk to build up the `_objects` array.
    //
    // This operation takes linear time in the number of serialized
    // objects, independent of their size, because the RIFF chunk
    // headers allow us to skip over the content of each of these
    // object-definition chunks.
    //
    _beginListChunk(RIFFSerial::kObjectDefinitionListFourCC);
    while (auto objectDefinitionChunk = _cursor.get())
    {
        ObjectInfo objectInfo;
        objectInfo.definitionChunk = objectDefinitionChunk;
        _objects.add(objectInfo);

        _advanceCursor();
    }
    _endListChunk();
}

void RIFFSerialReader::_flush()
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

        _cursor = deferredAction.savedCursor;
        deferredAction.callback(deferredAction.valuePtr, deferredAction.userData);
    }
}

FourCC RIFFSerialReader::_peekChunkType()
{
    auto chunk = _cursor.get();
    if (!chunk)
        return 0;
    return chunk->getType();
}

Int64 RIFFSerialReader::_readInt()
{
    switch (_peekChunkType())
    {
    case RIFFSerial::kInt64FourCC:
        return _readDataChunk<Int64>();
    case RIFFSerial::kInt32FourCC:
        return _readDataChunk<Int32>();

    case RIFFSerial::kUInt32FourCC:
        return _readDataChunk<UInt32>();

    case RIFFSerial::kUInt64FourCC:
        {
            auto uintValue = _readDataChunk<UInt64>();
            if (Int64(uintValue) < 0)
            {
                SLANG_UNEXPECTED("signed/unsigned mismatch in RIFF");
            }
            return Int64(uintValue);
        }

    default:
        SLANG_UNEXPECTED("invalid format in RIFF");
        UNREACHABLE_RETURN(0);
    }
}

UInt64 RIFFSerialReader::_readUInt()
{
    switch (_peekChunkType())
    {
    case RIFFSerial::kUInt64FourCC:
        return _readDataChunk<UInt64>();
    case RIFFSerial::kUInt32FourCC:
        return _readDataChunk<UInt32>();

    case RIFFSerial::kInt32FourCC:
    case RIFFSerial::kInt64FourCC:
        {
            auto intValue = _readInt();
            if (intValue < 0)
            {
                SLANG_UNEXPECTED("signed/unsigned mismatch in RIFF");
            }
            return UInt64(intValue);
        }

    default:
        SLANG_UNEXPECTED("invalid format in RIFF");
        UNREACHABLE_RETURN(0);
    }
}

double RIFFSerialReader::_readFloat()
{
    switch (_peekChunkType())
    {
    case RIFFSerial::kFloat32FourCC:
        return _readDataChunk<float>();
    case RIFFSerial::kFloat64FourCC:
        return _readDataChunk<double>();

    default:
        SLANG_UNEXPECTED("invalid format in RIFF");
        UNREACHABLE_RETURN(0);
    }
}

void RIFFSerialReader::_readDataChunk(void* outData, size_t dataSize)
{
    auto dataChunk = as<RIFF::DataChunk>(_cursor);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
        return;
    }
    auto size = dataChunk->getPayloadSize();
    if (size < dataSize)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
        return;
    }
    dataChunk->writePayloadInto(outData, dataSize);
    _advanceCursor();
}


void RIFFSerialReader::_beginListChunk(FourCC type)
{
    auto listChunk = as<RIFF::ListChunk>(_cursor);
    if (!listChunk || listChunk->getType() != type)
    {
        SLANG_UNEXPECTED("invalid format in RIFF");
    }

    _advanceCursor();
    _pushCursor();

    _cursor = listChunk->getFirstChild();
}

void RIFFSerialReader::_endListChunk()
{
    _popCursor();
}

void RIFFSerialReader::_advanceCursor()
{
    _cursor = _cursor.getNextSibling();
}

void RIFFSerialReader::_pushCursor()
{
    _stack.add(_cursor);
}

void RIFFSerialReader::_popCursor()
{
    SLANG_ASSERT(_stack.getCount() != 0);
    _cursor = _stack.getLast();
    _stack.removeLast();
}


} // namespace Slang
