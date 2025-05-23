// slang-serialize-fossil.h
#ifndef SLANG_SERIALIZE_FOSSIL_H
#define SLANG_SERIALIZE_FOSSIL_H

//
// This file provides implementations of `ISerializerImpl` that
// serialize hierarchical data in the "memory-mappable" binary
// format defined in `slang-fossil.h`.
//

#include "../core/slang-blob-builder.h"
#include "../core/slang-internally-linked-list.h"
#include "../core/slang-io.h"
#include "../core/slang-memory-arena.h"
#include "../core/slang-relative-ptr.h"
#include "slang-fossil.h"
#include "slang-serialize.h"

namespace Slang
{
namespace Fossil
{

/// Serializer implementation for writing objects to a fossil-format blob.
struct SerialWriter : ISerializerImpl
{
public:
    SerialWriter(ChunkBuilder* chunk);
    SerialWriter(BlobBuilder& blobBuilder);

    ~SerialWriter();

private:
    SerialWriter() = delete;

    void _initialize(ChunkBuilder* chunk);

    // The fossil format stores layout information, but that
    // information is kept separate from the values themselves.
    //
    // The nature of the `ISerializer` interface means that we
    // can only discover the layout as it is first being written,
    // so we need an intermediate representation of layouts
    // that we compute during the serialization process, before
    // we can write those layouts out as their own bytes.
    //
    // Two related issues make this task a little intricate:
    //
    // * We don't want to redundantly serialize many copies of
    //   the same layout (since the whole point of keeping the
    //   layout information separate from the content is to
    //   save on space), and ideally we don't want to *create*
    //   a large number of intermediate layouts that will end
    //   up getting deduplicated out of existence.
    //
    // * If the same C++ type is getting serialized multiple times
    //   (e.g., in a loop serializing an array) we both want to
    //   re-use the layout from the first element for subsequent
    //   elements *and* we want to handle the case where parts of
    //   the layout get expanded on subsequent iterations (e.g.,
    //   the first element in an array might have contained a null
    //   pointer, so there is no layout info for what it points to,
    //   but a later element might fill in that gap).
    //
    // The `_mergeLayouts()` operation is central to how these
    // issues are handled, allowing code to attach new information
    // to an existing layout as it goes.

    /// Representation of a layout for data that has been serialized.
    class LayoutObj
    {
    public:
        LayoutObj(FossilizedValKind kind, Size size = 0, Size alignment = 1)
            : kind(kind), size(size), alignment(alignment)
        {
        }

        virtual ~LayoutObj() {}

        FossilizedValKind getKind() const { return kind; }

        Size getSize() const { return size; }
        Size getAlignment() const { return alignment; }

        FossilizedValKind kind;
        Size size = 0;
        Size alignment = 1;

        /// If this layout is getting serialized out, then this
        /// is a pointer to the chunk that will store the `FossilizedValLayout`.
        ///
        ChunkBuilder* chunk = nullptr;
    };

    /// Create a layout of the given `kind`.
    ///
    /// If `kind` is one of the simple layout kinds, then this will
    /// return a singleton layout.
    ///
    LayoutObj* _createLayout(FossilizedValKind kind);

    LayoutObj* _createSimpleLayout(FossilizedValKind kind);
    Dictionary<FossilizedValKind, LayoutObj*> _simpleLayouts;

    // Rather than try to do detailed memory management for
    // layouts, we simply allocate them from an arena.

    MemoryArena _arena;

    /// Merge the `dst` layout object with the given `kind`.
    ///
    /// This more or less ensures that the layout *exists*
    /// and has the right kind.
    ///
    /// If `dst` is null, it will be initialized via `_createLayout`.
    /// 
    /// If `dst` is non-null, it will be checked against `kind`.
    ///
    LayoutObj* _mergeLayout(LayoutObj*& dst, FossilizedValKind kind);

    /// Merge the `src` layout into the `dst` layout.
    ///
    /// If `dst` is null, sets it to `src`.
    ///
    /// If `dst` is non-null, validates that `dst` and
    /// `src` have the same kind, and then may recursively
    /// merge their contents (e.g., if both are arrays,
    /// it will merge the element layouts).
    ///
    void _mergeLayout(LayoutObj*& dst, LayoutObj* src);

    /// Layout for simple types (integers, strings, etc.)
    class SimpleLayoutObj : public LayoutObj
    {
    public:
        SimpleLayoutObj(FossilizedValKind kind, Size size)
            : LayoutObj(kind, size, size)
        {
        }

        SimpleLayoutObj(FossilizedValKind kind)
            : LayoutObj(kind)
        {
        }
    };

    /// Layouts for objects that have one conceptual type parameter.
    ///
    /// The obvious cases include pointers, arrays, and optionals.
    /// 
    /// This is also used for dictionaries (the element type is
    /// a pair).
    ///
    /// This is also used for variants (the element type is the type
    /// of data that a *particular* variant used, whether or not
    /// it matches any others).
    ///
    class ContainerLayoutObj : public LayoutObj
    {
    public:
        ContainerLayoutObj(
            FossilizedValKind kind,
            LayoutObj* baseLayout,
            Size size = 0,
            Size alignment = 1)
            : LayoutObj(kind, size, alignment), baseLayout(baseLayout)
        {
        }

        LayoutObj* baseLayout = nullptr;
    };

    /// Layouts for tuples and structs.
    ///
    class RecordLayoutObj : public LayoutObj
    {
    public:
        RecordLayoutObj(FossilizedValKind kind)
            : LayoutObj(kind)
        {
        }

        struct FieldInfo
        {
            LayoutObj* layout = nullptr;
            Size offset = 0;
        };

        Count fieldCount = 0;
        Count fieldCapacity = 0;
        FieldInfo* fields = nullptr;
    };

    /// Get or add a field to the given `recordLayout` at the given `index`.
    ///
    /// If there is not already a field at `index`, then `index` must be
    /// equal to the number of existing fields.
    ///
    RecordLayoutObj::FieldInfo& _getOrAddField(RecordLayoutObj* layout, Index index);

    // The serialized representation only references layouts as part of
    // its encoding of variants, with each variant having a prefix field
    // that is a relative pointer to its serialized layout.
    //
    // Because we want to deduplicate layouts, we keep track of all of
    // the variant values we have serialized (each of which should be its
    // own chunk), and use that array to come back later and write out
    // their final layouts (after deduplication).

    struct VariantInfo
    {
        LayoutObj* layout = nullptr;
        ChunkBuilder* chunk = nullptr;
    };
    List<VariantInfo> _variants;

    /// Create a chunk to represent `layout`, or return a pre-existing one.
    ChunkBuilder* _getOrCreateChunkForLayout(LayoutObj* layout);

    /// Key for deduplication of `LayoutObj`s.
    struct LayoutObjKey
    {
        LayoutObjKey() {}

        LayoutObjKey(LayoutObj* obj)
            : obj(obj)
        {
        }

        LayoutObj* obj = nullptr;

        bool operator==(LayoutObjKey const& that) const;
        bool operator!=(LayoutObjKey const& that) const;

        HashCode64 getHashCode() const;
        void hashInto(Hasher& hasher) const;
    };
    Dictionary<LayoutObjKey, ChunkBuilder*> _mapLayoutObjToChunk;

    // We also go ahead and deduplicate strings as part of serialization,
    // since it is easy to do so.

    Dictionary<String, ChunkBuilder*> _mapStringToChunk;

    // Like almost any implementation of `ISerializer`, we need to track
    // information on the objects that have been encountered on the other
    // side of pointers, so that we can delay serializing their contents
    // until an appropriate time.

    struct FossilizedObjectInfo
    {
        /// Pointer to the "live" object.
        void* liveObjectPtr = nullptr;

        /// Chunk that will store the bytes of the fossilized object.
        ChunkBuilder* chunk = nullptr;

        /// Layout for a pointer to the fossilized `chunk`.
        LayoutObj* ptrLayout = nullptr;

        /// Callback information used by the ISerializer interface.
        Callback callback = nullptr;
        void* userData = nullptr;
    };

    List<FossilizedObjectInfo*> _fossilizedObjects;
    Dictionary<void*, FossilizedObjectInfo*> _mapLiveObjectPtrToFossilizedObject;
    Index _writtenObjectDefinitionCount = 0;

    /// Flush all pending operations.
    ///
    /// This function ensures that all of the to-be-writen objects have
    /// been written out, and that all of the variants that need a pointer
    /// to a serialized layout get one.
    ///
    void _flush();

    //
    // As the user makes various begin/end calls on this `SerialWriter`,
    // we need to push/pop state information so that we don't lose it.
    //

    struct State
    {
        /// The layout for the value being composed.
        LayoutObj* layout = nullptr;

        /// The number of elements/fields or other sub-values written so var.
        Count elementCount = 0;

        /// The chunk that holds the data for the value.
        ///
        /// Can be null if nothing has been written yet, in which
        /// case it may be allocated on teh first write.
        ///
        ChunkBuilder* chunk = nullptr;

        State() {}

        State(LayoutObj* layout, ChunkBuilder* chunk = nullptr)
            : layout(layout), chunk(chunk)
        {
        }
    };

    /// The current state.
    State _state;

    /// Stack of suspended states.
    List<State> _stack;

    /// The underlying blob builder that we are writing to.
    BlobBuilder* _blobBuilder = nullptr;

    //
    // Depending on the kind of value being written, it may
    // require a different representation. The `Val

    /// Represents a conceptual value to be written.
    ///
    /// Depending on the kind of value being written, it may
    /// require a different representation. The `ValInfo` type
    /// abstracts over these differences.
    ///
    /// Simple values that just consist of bytes can use the
    /// `RawData` case.
    ///
    /// Values that are encoded as a relative pointer use the
    /// `RelativePtr` case (unsurprisingly).
    ///
    /// The `ContentsOfChunk` case is used when the conceptual
    /// value is some kind of aggregate that is stored inline
    /// rather than indirectly.
    /// 
    struct ValInfo
    {
    public:
        enum class Kind
        {
            RawData,
            RelativePtr,
            ContentsOfChunk,
        };

        static ValInfo rawData(void const* data, Size size, Size alignment);
        static ValInfo relativePtrTo(ChunkBuilder* targetChunk);
        static ValInfo contentsOf(ChunkBuilder* chunk);

        Size getAlignment() const;

        Kind kind;
        union
        {
            struct
            {
                void const* ptr;
                Size size;
                Size alignment;
            } data;
            ChunkBuilder* chunk;
        };

    private:
        ValInfo() = default;
        ValInfo(const ValInfo&) = default;
        ValInfo(ValInfo&&) = default;
        ValInfo(Kind kind)
            : kind(kind)
        {
        }
    };

    // In order to allow building up layout information as values are
    // being written, the process of writing a value is broken into
    // two parts:
    //
    // * First, the code conceptually "reserves" a destination for the
    //   value it will write, passing in what it knows about the expected
    //   layout for the value. The reserve operation returns a layout
    //   to use (which may be a pre-existing one).
    //
    // * Second, once the value is ready as a `ValInfo`, the code "commits"
    //   the write and puts actual data in a chunk somewhere.
    //
    // For simple values these operations occur on after the other in
    // the same function. For complex things that need a begin/end pair,
    // the reserve usually happens in a `begin*()` or `push*()` function,
    // while the commit happens in an `end*()` or `pop*()` function.

    LayoutObj*& _reserveDestinationForWrite();
    LayoutObj* _reserveDestinationForWrite(FossilizedValKind srcKind);
    LayoutObj* _reserveDestinationForWrite(LayoutObj* srcLayout);

    void _commitWrite(ValInfo const& val);

    /// Write a value without doing any of the checks that `_commitWrite` does.
    ///
    /// (Usually this is called because `_commitWrite()` has already been called)
    void _writeValueRaw(ValInfo const& val);

    /// Ensure that the current `State` has a non-null chunk that data
    /// can be written to.
    ///
    void _ensureChunkExists();

    // There are various different categories of values that each
    // need slightly different handling, so each gets its own
    // operations that the various `ISerializer::begin()/end()`
    // functions will delegate to.
    //
    // The easiest case is simple values that consist of nothing
    // but plain data and have a layout that can be fully summarized
    // by the kind.

    void _writeSimpleValue(FossilizedValKind kind, void const* data, size_t size, size_t alignment);

    template<typename T>
    void _writeSimpleValue(FossilizedValKind kind, T const& value)
    {
        _writeSimpleValue(kind, &value, sizeof(value), sizeof(value));
    }

    /// Write a null (relative) pointer.
    /// 
    /// Use this case when there is no more refined type information
    /// available about what the layout of the pointed-to data *would*
    /// be if the pointer were non-null.
    ///
    void _writeNull();

    //
    // "Inline" values are aggregates like tuple and structs that
    // are always stored by-value in their parent.
    //

    void _pushInlineValueScope(FossilizedValKind kind);
    void _popInlineValueScope();

    //
    // "Indirect" values are those like optionals that are
    // stored as a pointer to an (optional) out-of-line value.
    //

    void _pushIndirectValueScope(FossilizedValKind kind);
    void _popIndirectValueScope();

    //
    // Many cases of values are *potentially* indirect, in that
    // they should be stored via pointer indirection *unless*
    // their immediate parent is something that already introduced
    // an indirection.
    //
    // A simple example is a string. A string will by default
    // be stored as a (relative) pointer to its content. However,
    // if there happens to be an *optional* string, then there is
    // no need for a second indirection.
    // 
    // Arrays, dictionaries, strings, and variants are all
    // potentially-indirect values.
    //
    // TODO: This is one aspect of the current design that may need
    // to be revisited, if it proves to add too much complexity.
    //

    void _pushPotentiallyIndirectValueScope(FossilizedValKind kind);
    ChunkBuilder* _popPotentiallyIndirectValueScope();

    /// Determine if a potentially-indirect value of `kind` should be
    /// emitted indirectly, in the current state.
    ///
    bool _shouldEmitWithPointerIndirection(FossilizedValKind kind);

    /// Helper function to share details between `_popIndirectValueScope`
    /// and `_popPotentiallyIndirectValueScope`.
    ///
    ChunkBuilder* _writeKnownIndirectValueSharedLogic(ChunkBuilder* valueChunk);

    //
    // Containers like arrays and dictionaries are potentially-indirect
    // values where the chunk that stores their content needs to
    // be given a prefix with the element count.
    //

    void _pushContainerScope(FossilizedValKind kind);
    void _popContainerScope();

    //
    // A variant is a potentially-indirect value where the chunk
    // that stores its content needs to be given a prefix with
    // the layout of the content.
    //

    void _pushVariantScope();
    void _popVariantScope();

    //
    // All of the above operations ultimately bottleneck through
    // `_pushState()`/`_popState()`.
    //

    void _pushState(LayoutObj* layout);
    void _popState();

private:

    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    virtual SerializationMode getMode() override;

    virtual void handleBool(bool& value) override;

    virtual void handleInt8(int8_t& value) override;
    virtual void handleInt16(int16_t& value) override;
    virtual void handleInt32(Int32& value) override;
    virtual void handleInt64(Int64& value) override;

    virtual void handleUInt8(uint8_t& value) override;
    virtual void handleUInt16(uint16_t& value) override;
    virtual void handleUInt32(UInt32& value) override;
    virtual void handleUInt64(UInt64& value) override;

    virtual void handleFloat32(float& value) override;
    virtual void handleFloat64(double& value) override;

    virtual void handleString(String& value) override;

    virtual void beginArray() override;
    virtual void endArray() override;

    virtual void beginOptional() override;
    virtual void endOptional() override;

    virtual void beginDictionary() override;
    virtual void endDictionary() override;

    virtual bool hasElements() override;

    virtual void beginTuple() override;
    virtual void endTuple() override;

    virtual void beginStruct() override;
    virtual void endStruct() override;

    virtual void beginVariant() override;
    virtual void endVariant() override;

    virtual void handleFieldKey(char const* name, Int index) override;

    virtual void handleSharedPtr(void*& value, Callback callback, void* userData) override;
    virtual void handleUniquePtr(void*& value, Callback callback, void* userData) override;

    virtual void handleDeferredObjectContents(void* valuePtr, Callback callback, void* userData)
        override;
};

/// Serializer implementation for reading objects from a fossil-format blob.
struct SerialReader : ISerializerImpl
{
public:
    SerialReader(FossilizedValRef valRef);
    ~SerialReader();

private:
    /// A state that the reader can be in.
    struct State
    {
        /// Type of state; related to the kind of value being read from.
        ///
        enum class Type
        {
            Root,
            Array,
            Dictionary,
            Optional,
            Tuple,
            Struct,
            Object,
        };

        /// The type of state.
        Type type = Type::Root;

        /// The fossilized value (data and layout) that is being read from.
        ///
        /// Depending on the `type` of state, this might either be the next value
        /// that will be read (e.g., for the `Root` case), or it might be
        /// a container that is a parent of the next value to be read.
        ///
        FossilizedValRef baseValue;

        /// Index of next element to read.
        ///
        /// This is used in the case where `baseValue` is some kind of
        /// container or record.
        ///
        Index elementIndex = 0;

        /// Total number of values that can be read.
        ///
        /// If `baseValue` is a container, this is the element count.
        /// If `baseValue` is a tuple/struct, this is the field count.
        /// If `baseValue` is an optional, this is either zero or one.
        /// If this state is a singleton case like `Root`, will be one.
        ///
        Count elementCount = 0;
    };

    /// The current state.
    State _state;

    /// Stack of saved states.
    List<State> _stack;

    void _pushState();
    void _popState();

    //
    // Like other `ISerializerImpl`s for reading, we track objects
    // that are in the process of being read in, to avoid possible
    // unbounded recursion (and detect circularities when they
    // occur).
    //

    enum class ObjectState
    {
        Unread,
        ReadingInProgress,
        ReadingComplete,
    };
    struct ObjectInfo : public RefObject
    {
        ObjectState state = ObjectState::Unread;

        void* resurrectedObjectPtr = nullptr;
        FossilizedValRef fossilizedObjectRef;
    };
    Dictionary<void*, RefPtr<ObjectInfo>> _mapFossilizedObjectPtrToObjectInfo;

    //
    // Again, like other `ISerializerImpl`s for reading, we
    // maintain a list of deferred serialization actions that
    // need to be performed to finish reading the state of
    // in-memory objects.
    //

    struct DeferredAction
    {
        void* resurrectedObjectPtr;

        State savedState;

        Callback callback;
        void* userData;
    };
    List<DeferredAction> _deferredActions;

    /// Execute all deferred actions that are still pending.
    void _flush();

    /// Read a simple/inline value.
    ///
    /// This is the case for scalars, tuples, and structs.
    ///
    FossilizedValRef _readValRef();

    /// Read an indirect value.
    ///
    /// This is the case for things like optionals, that are
    /// always encoded as a pointer.
    ///
    FossilizedValRef _readIndirectValRef();

    /// Read a potentially-indirect value.
    ///
    /// If the value that gets read is a pointer, then this
    /// function will return a reference to whatever it points to.
    ///
    /// Otherwise, this will return a reference to the value itself.
    ///
    FossilizedValRef _readPotentiallyIndirectValRef();

private:

    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    virtual SerializationMode getMode() override;

    virtual void handleBool(bool& value) override;

    virtual void handleInt8(int8_t& value) override;
    virtual void handleInt16(int16_t& value) override;
    virtual void handleInt32(Int32& value) override;
    virtual void handleInt64(Int64& value) override;

    virtual void handleUInt8(uint8_t& value) override;
    virtual void handleUInt16(uint16_t& value) override;
    virtual void handleUInt32(UInt32& value) override;
    virtual void handleUInt64(UInt64& value) override;

    virtual void handleFloat32(float& value) override;
    virtual void handleFloat64(double& value) override;

    virtual void handleString(String& value) override;

    virtual void beginArray() override;
    virtual void endArray() override;

    virtual void beginDictionary() override;
    virtual void endDictionary() override;

    virtual bool hasElements() override;

    virtual void beginStruct() override;
    virtual void endStruct() override;

    virtual void beginVariant() override;
    virtual void endVariant() override;

    virtual void handleFieldKey(char const* name, Int index) override;

    virtual void beginTuple() override;
    virtual void endTuple() override;

    virtual void beginOptional() override;
    virtual void endOptional() override;

    virtual void handleSharedPtr(void*& value, Callback callback, void* userData) override;
    virtual void handleUniquePtr(void*& value, Callback callback, void* userData) override;

    virtual void handleDeferredObjectContents(void* valuePtr, Callback callback, void* userData)
        override;
};

} // namespace Fossil
} // namespace Slang

#endif
