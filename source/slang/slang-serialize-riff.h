// slang-serialize-riff.h
#ifndef SLANG_SERIALIZE_RIFF_H
#define SLANG_SERIALIZE_RIFF_H

//
// This file provides implementations of `ISerializerImpl` that
// serialize hierarchical data in a RIFF-based format.
//
// This implementation can be seen as an adapter between the
// `Slang::Serializer` and `Slang::RIFF` subsystems, and also
// serves an an example of how to write a complete reader/writer
// pair for a new serialization format.
//

#include "../core/slang-riff.h"
#include "slang-serialize.h"

namespace Slang
{

namespace RIFFSerial
{
//
// Each value in the hierarchy will be ended as a RIFF chunk.
// The type of the chunk, and whether it is a list or data
// chunk will depend on the kind of value.
//
// All of the serialized data will be encapsulated in a single
// list chunk. This chunk can have its `FourCC` customized,
// but a default is also provided.
//

/// Default type for root chunk of a serialized object graph.
static const FourCC::RawValue kRootFourCC = SLANG_FOUR_CC('r', 'o', 'o', 't');

//
// Simple numeric values are stored as data chunks.
// Rather than go down to the granularity of 16- and
// 8-bit integers, we stick to 32- and 64-bit values
// only, since the overhead of a RIFF chunk header
// is already 64 bits (so the savings would be
// minimal).
//

static const FourCC::RawValue kInt32FourCC = SLANG_FOUR_CC('i', '3', '2', ' ');
static const FourCC::RawValue kInt64FourCC = SLANG_FOUR_CC('i', '6', '4', ' ');

static const FourCC::RawValue kUInt32FourCC = SLANG_FOUR_CC('u', '3', '2', ' ');
static const FourCC::RawValue kUInt64FourCC = SLANG_FOUR_CC('u', '6', '4', ' ');

static const FourCC::RawValue kFloat32FourCC = SLANG_FOUR_CC('f', '3', '2', ' ');
static const FourCC::RawValue kFloat64FourCC = SLANG_FOUR_CC('f', '6', '4', ' ');

//
// Boolean values are stored as empty chunks, with a unique
// type tag for each of the two possible values.
//

static const FourCC::RawValue kTrueFourCC = SLANG_FOUR_CC('t', 'r', 'u', 'e');
static const FourCC::RawValue kFalseFourCC = SLANG_FOUR_CC('f', 'a', 'l', 's');

//
// Strings are stored as a data chunk, with the payload of
// that chunk holding the bytes of the UTF-8 encoded string.
// The length of the string is stored as part of the chunk
// header.
//
// We also define a `FourCC` for raw data chunks, in anticipation
// of support for raw data being added to `ISerializerImpl` as
// an analogue of strings.
//

static const FourCC::RawValue kStringFourCC = SLANG_FOUR_CC('s', 't', 'r', ' ');
static const FourCC::RawValue kDataFourCC = SLANG_FOUR_CC('d', 'a', 't', 'a');

//
// Containers (arrays, dictionaries, optionals, tuples, and structs)
// are stored as list chunks, with their elements as child chunks.
//

static const FourCC::RawValue kArrayFourCC = SLANG_FOUR_CC('a', 'r', 'r', 'y');
static const FourCC::RawValue kDictionaryFourCC = SLANG_FOUR_CC('d', 'i', 'c', 't');
static const FourCC::RawValue kStructFourCC = SLANG_FOUR_CC('s', 't', 'r', 'c');
static const FourCC::RawValue kTupleFourCC = SLANG_FOUR_CC('t', 'p', 'l', 'e');
static const FourCC::RawValue kOptionalFourCC = SLANG_FOUR_CC('o', 'p', 't', '?');

//
// Null pointer values are simply stored as an empty data chunk with
// a distinct type.
//

static const FourCC::RawValue kNullFourCC = SLANG_FOUR_CC('n', 'u', 'l', 'l');

//
// Non-null pointers are stored as a data chunk that references a
// serialized object by its `ObjectIndex`.
//

using ObjectIndex = Int32;

static const FourCC::RawValue kObjectReferenceFourCC = SLANG_FOUR_CC('o', 'b', 'j', 'r');

//
// All of the objects transitively referenced in the serialized object
// graph are stored in a list chunk of object definitions, with one
// chunk per object. The object definitions themselves are ordinary
// values using any of the cases above.
//

static const FourCC::RawValue kObjectDefinitionListFourCC = SLANG_FOUR_CC('o', 'b', 'j', 's');

//
// The first child of the root chunk will be the object definition list
// chunk, and that will be followed by zero or more "root values" that
// have been serialized.
//

} // namespace RIFFSerial

/// Serializer implementation for writing to a tree of RIFF chunks.
struct RIFFSerialWriter
{
public:
    /// Construct a writer to append to the given RIFF `chunk`.
    ///
    /// The object graph will be serialized into a child chunk
    /// of `chunk`, as a list chunk with the given `type`.
    ///
    RIFFSerialWriter(RIFF::ChunkBuilder* chunk, FourCC type = RIFFSerial::kRootFourCC);


    /// Construct a writer to write an entire RIFF file.
    ///
    /// The object graph will be serialized as the root chunk
    /// of `riff`, with the given `type`.
    ///
    RIFFSerialWriter(RIFF::Builder& riff, FourCC type = RIFFSerial::kRootFourCC);

    /// Finalize writing.
    ///
    /// Any pending operations needed to write the entire object
    /// graph will be flushed.
    ///
    ~RIFFSerialWriter();

private:
    RIFFSerialWriter() = delete;

    /// Cursor for where in the RIFF hierarchy we are writing.
    RIFF::BuildCursor _cursor;

    /// Representation of an index into the object list.
    using ObjectIndex = RIFFSerial::ObjectIndex;

    /// Information about an object that should be
    /// added to the object definition list.
    struct ObjectInfo
    {
        /// Pointer to the in-memory C++ object.
        void* ptr;

        /// Callback that can be invoked to serialize the object's data.
        SerializerCallback callback;

        /// Context pointer for `callback`
        void* context;
    };

    /// The chunk where object definitions are listed.
    RIFF::ListChunkBuilder* _objectDefinitionListChunk = nullptr;

    /// Information on the objects that have been referenced,
    /// and which need their definitions to be serialized into
    /// the object definition list chunk.
    ///
    List<ObjectInfo> _objects;
    Index _writtenObjectDefinitionCount = 0;

    /// Maps the address of an in-memory C++ object to the
    /// corresponding entry in `_objects`, if any.
    Dictionary<void*, ObjectIndex> _mapPtrToObjectIndex;


    void _initialize(FourCC type);
    void _flush();

    void _writeInt(Int64 value);
    void _writeUInt(UInt64 value);
    void _writeFloat(double value);

    void _writeObjectReference(ObjectIndex index);

private:
    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    SerializationMode getMode();

    void handleBool(bool& value);

    void handleInt8(int8_t& value);
    void handleInt16(int16_t& value);
    void handleInt32(Int32& value);
    void handleInt64(Int64& value);

    void handleUInt8(uint8_t& value);
    void handleUInt16(uint16_t& value);
    void handleUInt32(UInt32& value);
    void handleUInt64(UInt64& value);

    void handleFloat32(float& value);
    void handleFloat64(double& value);

    void handleString(String& value);

    struct Scope
    {
        // The RIFF serialization back-end is currently
        // not taking advantage of the `Scope` facility
        // in the serialization framework.
    };

    void beginArray(Scope&);
    void endArray(Scope&);

    void beginDictionary(Scope&);
    void endDictionary(Scope&);

    bool hasElements();

    void beginStruct(Scope&);
    void endStruct(Scope&);

    void beginVariant(Scope&);
    void endVariant(Scope&);

    void handleFieldKey(char const* name, Int index);

    void beginTuple(Scope&);
    void endTuple(Scope&);

    void beginOptional(Scope&);
    void endOptional(Scope&);

    void handleSharedPtr(void*& value, SerializerCallback callback, void* context);
    void handleUniquePtr(void*& value, SerializerCallback callback, void* context);

    void handleDeferredObjectContents(void* valuePtr, SerializerCallback callback, void* context);
};

/// Serializer implementation for reading from a tree of RIFF chunks.
struct RIFFSerialReader
{
public:
    /// Construct a reader to read data from the given `chunk`.
    ///
    /// Will validate that the given `chunk` is a list chunk
    /// matching the expected `type`.
    ///
    RIFFSerialReader(RIFF::Chunk const* chunk, FourCC type = RIFFSerial::kRootFourCC);

    /// Finalize the reader.
    ///
    /// This will flush any outstanding operations that
    /// might be pending.
    ///
    ~RIFFSerialReader();

    /// Read a chunk from the current cursor position.
    ///
    /// This operation can be used to skip over an entire value
    /// that might otherwise need to be read with a sequence of
    /// operations of the `ISerializerImpl` interface.
    ///
    /// The saved pointer can then be used to construct another
    /// `RIFFSerialReader` to read the contents of the chunk
    /// at some later time, or code can simply navigate the
    /// chunk in memory using their own logic.
    ///
    RIFF::Chunk const* readChunk();

private:
    /// Representation of a read cursor in the serialized RIFF data.
    using Cursor = RIFF::BoundsCheckedChunkPtr;

    /// Current cursor in the serialized RIFF data.
    Cursor _cursor;

    void _advanceCursor();

    /// A stack of saved cursors, reflecting the nesting
    /// hierarchy of container chunks being read from.
    ///
    List<Cursor> _stack;

    void _pushCursor();
    void _popCursor();

    /// Representation of an index into the object list.
    using ObjectIndex = RIFFSerial::ObjectIndex;

    /// State of a serialized object that may or may not have been read already.
    enum class ObjectState
    {
        Unread,
        ReadingInProgress,
        ReadingComplete,
    };

    /// Information about a serialized object in the object definition list.
    struct ObjectInfo
    {
        /// State of the object.
        ///
        ObjectState state = ObjectState::Unread;

        /// Pointer to an in-memory C++ object representing the serialized object.
        ///
        /// Should only be accessed with consideration of what `state` is:
        ///
        /// * If `state` is `Unread`, then this should always be null.
        ///
        /// * If `state` is `ReadingComplete`, then this should be be
        ///   a valid pointer to the in-memory representation of the serialized object
        ///   (or null, if client code chose to deserialize it into a null pointer
        ///   for some reason).
        ///
        /// * If `state` is `ReadingInProgress`, then this might be a null pointer,
        ///   indicating that the logic to deserialize the object is currently
        ///   running (but has not yet allocated a representation and set it), or
        ///   it might be non-null indicating that the in-memory representation
        ///   has been allocated.
        ///
        /// Even if `ptr` is non-null, it may not be safe to access the
        /// contents of the pointed-to object, because there may be deferred
        /// operations pending to read some or all of its members.
        ///
        void* ptr = nullptr;

        /// The chunk that holds the definition of this object.
        RIFF::Chunk const* definitionChunk = nullptr;
    };

    /// All of the objects from the object definition list chunk.
    List<ObjectInfo> _objects;

    /// A serialization action that has been deferred.
    ///
    /// Deferred actions are typically used to put off recursively
    /// reading all of the members of an object, thus avoiding
    /// the potential for unbounded or even infinite recursion.
    ///
    struct DeferredAction
    {
        /// The in-memory object that the action should apply to.
        void* valuePtr;

        /// The value of `_cursor` at the time this action was deferred.
        Cursor savedCursor;

        /// The callback to apply to read data into the `valuePtr`
        SerializerCallback callback;

        /// The context pointer for the `callback`.
        void* context;
    };

    /// Deferred actions that are still pending.
    ///
    /// As long as this array is non-empty, the contents of
    /// in-memory objects read from the serialized data should
    /// not be inspected/used.
    ///
    List<DeferredAction> _deferredActions;

    void _initialize(FourCC type);
    void _flush();

    FourCC _peekChunkType();

    Int64 _readInt();
    UInt64 _readUInt();
    double _readFloat();

    ObjectIndex _readObjectReference();

    void _readDataChunk(void* outData, size_t dataSize);

    template<typename T>
    T _readDataChunk()
    {
        T value;
        _readDataChunk(&value, sizeof(value));
        return value;
    }

    void _beginListChunk(FourCC type);
    void _endListChunk();

private:
    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    SerializationMode getMode();

    void handleBool(bool& value);

    void handleInt8(int8_t& value);
    void handleInt16(int16_t& value);
    void handleInt32(Int32& value);
    void handleInt64(Int64& value);

    void handleUInt8(uint8_t& value);
    void handleUInt16(uint16_t& value);
    void handleUInt32(UInt32& value);
    void handleUInt64(UInt64& value);

    void handleFloat32(float& value);
    void handleFloat64(double& value);

    void handleString(String& value);

    struct Scope
    {
        // The RIFF serialization back-end is currently
        // not taking advantage of the `Scope` facility
        // in the serialization framework.
    };

    void beginArray(Scope&);
    void endArray(Scope&);

    void beginDictionary(Scope&);
    void endDictionary(Scope&);

    bool hasElements();

    void beginStruct(Scope&);
    void endStruct(Scope&);

    void beginVariant(Scope&);
    void endVariant(Scope&);

    void handleFieldKey(char const* name, Int index);

    void beginTuple(Scope&);
    void endTuple(Scope&);

    void beginOptional(Scope&);
    void endOptional(Scope&);

    void handleSharedPtr(void*& value, SerializerCallback callback, void* context);
    void handleUniquePtr(void*& value, SerializerCallback callback, void* context);

    void handleDeferredObjectContents(void* valuePtr, SerializerCallback callback, void* context);
};

} // namespace Slang

#endif
