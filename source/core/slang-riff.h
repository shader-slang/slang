// slang-riff.h
#ifndef SLANG_RIFF_H
#define SLANG_RIFF_H

// This file defines an API for reading and writing files in the
// RIFF file format.
//
// Some references on the RIFF format include:
//
// * http://fileformats.archiveteam.org/wiki/RIFF
// * http://www.fileformat.info/format/riff/egff.htm
//
// RIFF files, and formats inspired by it, are commonly used as
// binary interchange formats in cases where ad hoc extensibility
// is needed.
//

#include "slang-basic.h"
#include "slang-memory-arena.h"
#include "slang-stream.h"
#include "slang-writer.h"

namespace Slang
{

//
// An important concept in the RIFF format, as well as
// many derived formats, is a *four-character code*, usually
// referred to as a "FourCC" or "FOURCC".
//

/// A 32-bit value that comprises four ASCII characters.
///
/// A `FourCC` can be used as a kind of "extensible `enum`" in situations
/// where different developers or groups may want to independently add
/// cases, while minimizing the chances of accidental collisions.
///
/// A `FourCC` can be efficienctly compared, or used in `switch`
/// statements, which can be an advantage compared to alternative
/// extensible formats like strings or UUIDs.
///
/// In memory, the characters of a `FourCC` come in the
/// usual order that they would for an array of four `char`s;
/// that is, the first character occupies the byte with the
/// lowest address, and so forth. When that same memory
/// is read as a 32-bit integer, the integer value read will
/// depend on the endianness of the architecture.
///
struct FourCC
{
public:
    /// The value of a `FourCC`, represented as single integer.
    using RawValue = UInt32;

    FourCC() { _rawValue = 0; }

    FourCC(RawValue rawValue) { _rawValue = rawValue; }

    void operator=(RawValue rawValue) { _rawValue = rawValue; }

    RawValue getRawValue() const { return _rawValue; }

    operator RawValue() const { return _rawValue; }

private:
    //
    // The storage for a `FourCC` is defined in a
    // way that makes the textual form more visible
    // when debugging.
    //
    union
    {
        char _text[4];
        RawValue _rawValue;
    };
};

//
// Because the integer representation of a `FourCC` depends
// on the endianness of the architecture, we define a macro
// to turn a sequence of four independent characters into
// a single `FourCC::RawValue`, based on the target
// architecture.
//

#if SLANG_LITTLE_ENDIAN

#define SLANG_FOUR_CC(c0, c1, c2, c3)                                                           \
    ((FourCC::RawValue(c0) << 0) | (FourCC::RawValue(c1) << 8) | (FourCC::RawValue(c2) << 16) | \
     (FourCC::RawValue(c3) << 24))

#else

#define SLANG_FOUR_CC(c0, c1, c2, c3)                                                            \
    ((FourCC::RawValue(c0) << 24) | (FourCC::RawValue(c1) << 16) | (FourCC::RawValue(c2) << 8) | \
     (FourCC::RawValue(c3) << 0))
#endif


namespace RIFF
{

struct Chunk;
struct DataChunk;
struct ListChunk;
struct RootChunk;
class ChunkBuilder;
class ListChunkBuilder;
class DataChunkBuilder;
struct Builder;

//
// A RIFF file is organized as a tree of *chunks*.
//

/// A chunk in a RIFF file.
///
struct Chunk
{
public:
    //
    // The starting offset of a chunk in a RIFF file
    // is only guaranteed to be 2-byte aligned.
    // Code that reads from a chunk must be cautious about
    // the possibility of performing unaligned loads.
    //

    /// Required alignment for the starting offset of a chunk.
    static const UInt32 kChunkAlignment = 2;

    //
    // Every chunk starts with a *header*, which includes
    // a *tag* used to identify the kind/type of chunk
    // as well a representation of the size of the chunk
    // in bytes.
    //
    struct Header
    {
        //
        // Note that when loading a RIFF file into memory,
        // chunks may not start on 4-byte-aligned boundaries,
        // so code should not directly read the following
        // fields unless preconditions exist to guarantee
        // higher alignment.
        //

        /// Tag for this chunk.
        ///
        /// * For a data chunk, this will be its type.
        /// * For a list chunk this will be `"LIST"`.
        /// * For a root chunk this will be `"RIFF"`.
        ///
        FourCC tag;

        /// Size in bytes of this chunk, not including this header.
        UInt32 size;
    };

    /// Get the header for this chunk.
    Header const* getHeader() const { return (Header const*)this; }

    /// Get the tag from the header of this chunk.
    FourCC getTag() const { return _readTagFromHeader(); }

    /// Get the total size of this chunk, in bytes.
    ///
    /// This size includes the chunk header.
    ///
    UInt32 getTotalSize() const { return sizeof(RIFF::Chunk) + _readSizeFromHeader(); }

    //
    // There are three *kinds* of chunks that can appear in a RIFF:
    //
    // * *data chunks* contain zero or more bytes of data.
    //
    // * *list chunks* contain a sequence of other chunks
    //
    // * a *root chunk* is a special case of list chunk that
    //   is used as the root of the chunk hierarchy in a RIFF file.
    //
    // List chunks are identified by having a tag of `"LIST"`
    // in their header, while root chunks have a tag of `"RIFF"`
    //

    /// Kind of a chunk.
    enum class Kind
    {
        Data,
        List,
        Root,
    };

    /// Get the kind of this chunk.
    Kind getKind() const;

    //
    // Every chunk has a *type*, which is a `FourCC`.
    //
    // For data chunks, the type is stored as the tag
    // of the chunk header, while for list and root
    // chunks the type is stored immediately after
    // the chunk header.
    //

    /// Type of a chunk.
    using Type = FourCC;

    /// Get the type of this chunk.
    Type getType() const;

private:
    Header _header;

protected:
    //
    // Rather than directly reading the `_tag` or `_size`
    // members, code should use the following accessors,
    // which account for possible alignment issues.
    //

    FourCC _readTagFromHeader() const
    {
        auto header = getHeader();
        FourCC result;
        memcpy(&result, &header->tag, sizeof(header->tag));
        return result;
    }

    UInt32 _readSizeFromHeader() const
    {
        auto header = getHeader();
        UInt32 result;
        memcpy(&result, &header->size, sizeof(header->size));
        return result;
    }
};

/// A chunk that contains zero or more bytes of payload data.
struct DataChunk : Chunk
{
public:
    /// Get the size in bytes of the payload data of this chunk.
    UInt32 getPayloadSize() const { return _readSizeFromHeader(); }

    /// Get a pointer to the payload data of this chunk.
    ///
    /// Note that this pointer is only guaranteed to be aligned
    /// up to `RIFF::Chunk::kAlignment`, for chunks of a RIFF
    /// file loaded directly into memory.
    ///
    void const* getPayload() const { return static_cast<void const*>(this + 1); }

    /// Write the payload data of this chunk into the given buffer.
    ///
    /// The payload must be at least `size` bytes.
    /// If the payload is larger than `size` bytes, only the
    /// first `size` bytes will be written to the buffer.
    ///
    void writePayloadInto(void* outData, Size size) const;

    /// Get the payload data of this chunk.
    ///
    template<typename T>
    void writePayloadInto(T& outValue) const
    {
        writePayloadInto(&outValue, sizeof(outValue));
    }

    /// Get the payload data of this chunk.
    ///
    template<typename T>
    T readPayloadAs() const
    {
        T result;
        writePayloadInto(result);
        return result;
    }

    /// Get the type of this chunk.
    Type getType() const
    {
        // The type of a data chunk is just the tag
        // from the chunk header.

        return getTag();
    }

    /// Determine if a chunk is an instance of this kind.
    static bool _isChunkOfThisKind(Chunk const* chunk)
    {
        return chunk->getKind() == Chunk::Kind::Data;
    }
};

/// A pointer to a `RIFF::Chunk` that is dynamically
/// checked to make sure access doesn't go past a
/// certain size bound.
///
struct BoundsCheckedChunkPtr
{
public:
    /// Initialize a null pointer
    BoundsCheckedChunkPtr() {}

    /// Initialize a null pointer
    BoundsCheckedChunkPtr(nullptr_t) {}


    /// Initialize a pointer to a chunk, with a size limit.
    BoundsCheckedChunkPtr(Chunk const* chunk, Size sizeLimit) { _set(chunk, sizeLimit); }

    /// Initialize a pointer to a chunk, with a limit based on its reported size.
    BoundsCheckedChunkPtr(Chunk const* chunk) { _set(chunk); }

    /// Get the underlying chunk pointer.
    Chunk const* get() const { return _ptr; }

    operator Chunk const*() const { return get(); }
    Chunk const* operator->() const { return get(); }

    BoundsCheckedChunkPtr getNextSibling() const;

private:
    Chunk const* _ptr = nullptr;
    Size _sizeLimit = 0;

    void _set(Chunk const* chunk, Size sizeLimit);
    void _set(Chunk const* chunk);
};


template<typename T = Chunk>
struct ChunkList
{
public:
    ChunkList() {}

    ChunkList(BoundsCheckedChunkPtr firstChunk)
        : _firstChunk(firstChunk)
    {
    }

    struct Iterator
    {
    public:
        Iterator() {}

        Iterator(BoundsCheckedChunkPtr chunk)
            : _chunk(chunk)
        {
        }

        T const* operator*() const { return static_cast<T const*>(_chunk.get()); }

        void operator++() { _chunk = _chunk.getNextSibling(); }

        bool operator!=(Iterator const& that) const { return _chunk != that._chunk; }

    private:
        BoundsCheckedChunkPtr _chunk;
    };

    Iterator begin() const { return Iterator(_firstChunk); }
    Iterator end() const { return Iterator(); }

    template<typename U>
    ChunkList<U> cast() const
    {
        return ChunkList<U>(_firstChunk);
    }

    T const* getFirst() const { return *begin(); }

private:
    friend struct ListChunk;

    BoundsCheckedChunkPtr _firstChunk;
};

struct ListChunk : Chunk
{
public:
    //
    // A (non-root) list chunk has a tag of `"LIST"`
    // in its header.
    //

    static const FourCC::RawValue kTag = SLANG_FOUR_CC('L', 'I', 'S', 'T');

    //
    // A list chunk starts with a header, as all chunks do,
    // but for a list chunk the ordinary `Chunk::Header`
    // is followed by an additional `FourCC`, to specify
    // the type of the list chunk.
    //

    struct Header
    {
        //
        // As is the case with any other chunk, code that
        // wants to access these fields should be mindful
        // of the way that a RIFF does not guarantee 4-byte
        // alignment for chunks.
        //

        /// The base chunk header.
        Chunk::Header chunkHeader;

        /// The type of this list chunk.
        Type type;
    };

    /// Get the header for this list chunk.
    Header const* getHeader() const { return (Header const*)this; }

    //
    // The content of a list chunk comprises zero or more
    // child chunks, organized as a kind of linked list.
    //
    // The starting offset for each successive child chunk
    // is the end offset of the previous child chunk, rounded
    // up to the required alignment (`RIFF::Chunk::kAlignment`).
    //

    /// List of child chunks.
    using ChildList = ChunkList<>;

    /// Get the list of children of this chunk.
    ChildList getChildren() const { return ChildList(getFirstChild()); }

    /// Get the first child chunk (if any) of this chunk.
    ///
    /// The list of child chunks can be navigated using
    /// the `BoundCheckedChunkPtr::getNextSibling` operation.
    ///
    BoundsCheckedChunkPtr getFirstChild() const;

    /// Find a child data chunk of the given `type`.
    DataChunk const* findDataChunk(Chunk::Type type) const;

    /// Find a child list chunk of the given `type`.
    ListChunk const* findListChunk(Chunk::Type type) const;

    /// Recursively search for a list chunk of the given `type`.
    ///
    /// Will consider this chunk itself as a possible match.
    ///
    ListChunk const* findListChunkRec(Chunk::Type type) const;

    /// Get the type of this chunk.
    Type getType() const { return _readTypeFromHeader(); }

    /// Determine if a chunk is an instance of this kind.
    static bool _isChunkOfThisKind(Chunk const* chunk)
    {
        // Anything that isn't a data chunk is a list.
        return chunk->getKind() != Chunk::Kind::Data;
    }

private:
    //
    // Because we are inheriting from `Chunk`, we do not
    // declare a full `ListChunk::Header` here, and instead
    // just declare the additional field that appears after
    // the base header.
    //

    Type _type;

    //
    // The `_type` field is mostly just there for debugging
    // purposes; when actually reading from the header, we
    // make use of a cast.
    //

    Type _readTypeFromHeader() const
    {
        auto header = getHeader();
        Type result;
        memcpy(&result, &header->type, sizeof(header->type));
        return result;
    }
};

struct RootChunk : ListChunk
{
public:
    //
    // A root chunk has a tag of `"RIFF"` in its header.
    //

    static const FourCC::RawValue kTag = SLANG_FOUR_CC('R', 'I', 'F', 'F');

    /// Get a pointer to the root chunk of a RIFF hierarchy stored in a data blob.
    ///
    /// Performs some minimal validity checks, and returns `nullptr` if
    /// the blob provided does not superficially appear to be a valid RIFF.
    ///
    static RootChunk const* getFromBlob(void const* data, size_t dataSize);

    /// Get a pointer to the root chunk of a RIFF hierarchy stored in a data blob.
    ///
    /// Performs some minimal validity checks, and returns `nullptr` if
    /// the blob provided does not superficially appear to be a valid RIFF.
    ///
    static RootChunk const* getFromBlob(ISlangBlob* blob);

    /// Determine if a chunk is an instance of this kind.
    static bool _isChunkOfThisKind(Chunk const* chunk)
    {
        return chunk->getKind() == Chunk::Kind::Root;
    }

private:
    static bool _isTagForThisKind(FourCC tag) { return tag == kTag; }
};

inline Chunk::Kind Chunk::getKind() const
{
    switch (getTag())
    {
    case RootChunk::kTag:
        return Chunk::Kind::Root;
    case ListChunk::kTag:
        return Chunk::Kind::List;
    default:
        return Chunk::Kind::Data;
    }
}

inline Chunk::Type Chunk::getType() const
{
    auto tag = getTag();
    switch (tag)
    {
    case RootChunk::kTag:
    case ListChunk::kTag:
        return static_cast<ListChunk const*>(this)->getType();

    default:
        return tag;
    }
}

/// Cast a `Chunk` to a sub-type of `Chunk`.
template<typename T>
T* as(Chunk* chunk)
{
    if (!chunk)
        return nullptr;
    if (!T::_isChunkOfThisKind(chunk))
        return nullptr;
    return static_cast<T*>(chunk);
}

/// Cast a `Chunk` to a sub-type of `Chunk`.
template<typename T>
T const* as(Chunk const* chunk)
{
    if (!chunk)
        return nullptr;
    if (!T::_isChunkOfThisKind(chunk))
        return nullptr;
    return static_cast<T const*>(chunk);
}

/// A linked list where the elements are themselves the nodes.
///
template<typename T>
struct InternallyLinkedList
{
public:
    struct Node
    {
    public:
        Node() {}

    private:
        friend struct InternallyLinkedList<T>;
        T* _next = nullptr;
    };

    struct Iterator
    {
    public:
        Iterator() {}

        Iterator(T* node)
            : _node(node)
        {
        }

        T* operator*() const { return _node; }

        void operator++() { _node = static_cast<Node const*>(_node)->_next; }

        bool operator!=(Iterator const& that) const { return _node != that._node; }

    private:
        T* _node = nullptr;
    };

    Iterator begin() { return Iterator(_first); }

    Iterator end() { return Iterator(); }

    T* getFirst() const { return _first; }

    T* getLast() const { return _last; }

    void add(T* element)
    {
        if (!_last)
        {
            SLANG_ASSERT(_first == nullptr);

            _first = element;
            _last = element;
        }
        else
        {
            SLANG_ASSERT(_first != nullptr);

            _last->_next = element;
            _last = element;
        }
    }

private:
    T* _first = nullptr;
    T* _last = nullptr;
};

/// A builder for a chunk in a RIFF.
class ChunkBuilder : public InternallyLinkedList<ChunkBuilder>::Node
{
public:
    /// Get the kind of the chunk being built.
    Chunk::Kind getKind() const { return _kind; }

    /// Get the type of the chunk being built.
    Chunk::Type getType() const { return _type; }

    /// Set the type of the chunk being built.
    void setType(Chunk::Type type) { _type = type; }

    /// Get the parent chunk of this chunk in the RIFF hierarchy.
    ///
    ListChunkBuilder* getParent() const { return _parent; }

    /// Get the RIFF builder that this chunk belongs to.
    ///
    RIFF::Builder* getRIFFBuilder() const { return _riffBuilder; }

protected:
    ChunkBuilder(
        Chunk::Kind kind,
        Chunk::Type type,
        ListChunkBuilder* parent,
        RIFF::Builder* riffBuilder)
        : _kind(kind), _type(type), _parent(parent), _riffBuilder(riffBuilder)
    {
    }

    ChunkBuilder(ChunkBuilder const&) = delete;
    void operator=(ChunkBuilder const&) = delete;

    MemoryArena& _getMemoryArena() const;

private:
    Chunk::Kind _kind = Chunk::Kind(-1);
    Chunk::Type _type = 0;
    ListChunkBuilder* _parent = nullptr;
    Builder* _riffBuilder = nullptr;

    // A cached total size for this chunk. This
    // is only valid after `_updateCachedTotalSize()`
    // has been called, and before any subsequent
    // changes to the content of this chunk or any
    // of its descendents in the hierarchy.
    //
    mutable Size _cachedTotalSize = 0;

    Size _updateCachedTotalSize() const;

    Size _getCachedTotalSize() const { return _cachedTotalSize; }

    /// Write the binary representation of this chunk to the given `stream`
    ///
    /// Assumes that `_updateCachedTotalSize` has been used
    /// so that the cached total size of this chunk is valid.
    ///
    Result _writeTo(Stream* stream) const;

    friend struct Builder;
};

class ListChunkBuilder : public ChunkBuilder
{
public:
    /// A list of child chunks.
    using ChildList = InternallyLinkedList<ChunkBuilder>;

    /// Get the child chunks of this list.
    ChildList getChildren() const { return _children; }

    /// Append a new data chunk to the current list chunk.
    DataChunkBuilder* addDataChunk(Chunk::Type type);

    /// Append a new data chunk to the current list chunk.
    ListChunkBuilder* addListChunk(Chunk::Type type);

    /// Determine if a chunk is an instance of this kind.
    static bool _isChunkOfThisKind(ChunkBuilder const* chunk)
    {
        return chunk->getKind() != Chunk::Kind::Data;
    }

private:
    ListChunkBuilder(Chunk::Type type, ListChunkBuilder* parent)
        : ChunkBuilder(Chunk::Kind::List, type, parent, parent->getRIFFBuilder())
    {
    }

    friend struct RIFF::Builder;

    ListChunkBuilder(Chunk::Type type, RIFF::Builder* riffBuilder)
        : ChunkBuilder(Chunk::Kind::Root, type, nullptr, riffBuilder)
    {
    }

    ChildList _children;
};


/// A builder for a data chunk in a RIFF.
class DataChunkBuilder : public ChunkBuilder
{
public:
    /// Append data to this chunk.
    void addData(void const* data, Size size);

    /// Append data to this chunk.
    template<typename T>
    void addData(T const& value)
    {
        addData(&value, sizeof(value));
    }

    /// Append existing data to this chunk.
    ///
    /// The caller takes responsibility for ensuring that
    /// the passed-in data pointer will remain valid for
    /// the rest of the lifetime of the enclosing RIFF
    /// builder.
    ///
    void addUnownedData(void const* data, size_t size);

    //
    // While the payload of a chunk in a RIFF file is
    // contiguous, the payload of a `DataChunkBuilder`
    // can span multiple different allocations, which
    // this implementation refers to as *shards*.
    //
    // Each shard has a contiguous payload, and the
    // `DataChunkBuilder` owns a list of shards. The
    // logical payload of the data chunk is the
    // concatenation of the payloads of its shards.
    //

    /// A contiguous range of bytes in a `RIFF::DataChunkBuilder`
    class Shard : public InternallyLinkedList<Shard>::Node
    {
    public:
        /// Get the payload of this shard.
        void const* getPayload() const { return _payload; }

        /// Get the size of the payload of this shard.
        Size getPayloadSize() const { return _payloadSize; }

    private:
        friend class DataChunkBuilder;

        Shard() {}

        void setPayload(void const* data, Size size)
        {
            _payload = data;
            _payloadSize = size;
        }

        void const* _payload = nullptr;
        Size _payloadSize = 0;
    };

    /// List of shards in a data chunk.
    using ShardList = InternallyLinkedList<Shard>;

    /// Get the list of shards that make up this chunk.
    ShardList getShards() const { return _shards; }

    /// Determine if a chunk is an instance of this kind.
    static bool _isChunkOfThisKind(ChunkBuilder const* chunk)
    {
        return chunk->getKind() == Chunk::Kind::Data;
    }

private:
    friend class ListChunkBuilder;

    DataChunkBuilder(Chunk::Type type, ListChunkBuilder* parent)
        : ChunkBuilder(Chunk::Kind::Data, type, parent, parent->getRIFFBuilder())
    {
    }

    Shard* _addShard();

    ShardList _shards;
};

template<typename T>
T* as(ChunkBuilder* chunk)
{
    if (!chunk)
        return nullptr;
    if (!T::_isChunkOfThisKind(chunk))
        return nullptr;
    return static_cast<T*>(chunk);
}

template<typename T>
T const* as(ChunkBuilder const* chunk)
{
    if (!chunk)
        return nullptr;
    if (!T::_isChunkOfThisKind(chunk))
        return nullptr;
    return static_cast<T const*>(chunk);
}

/// A builder for a RIFF-structured file.
///
struct Builder
{
public:
    /// Initialize a builder with an empty tree of chunks.
    Builder();

    /// Write the built hierarchy out to the given `stream`.
    Result writeTo(Stream* stream);

    /// Write the built hierarchy out as a blob.
    Result writeToBlob(ISlangBlob** outBlob);

    /// Get the root chunk of the RIFF being built.
    ///
    /// If a root chunk has not yet been added, returns `nullptr`.
    ///
    ListChunkBuilder* getRootChunk() const { return _rootChunk; }

    /// Add a root chunk to the RIFF being built.
    ///
    /// There must not already be a root chunk.
    ///
    /// Returns the root chunk that was added.
    ///
    ListChunkBuilder* addRootChunk(Chunk::Type type);

    /// Get the memory arena used for allocation.
    ///
    /// This arena is used for allocating all of the chunk
    /// builders, as well as their data.
    ///
    /// Note: typical use cases should never need to
    /// access this; it is part of the public API
    /// primarily to enable some of the unit tests.
    ///
    MemoryArena& _getMemoryArena() { return _arena; }

private:
    Builder(Builder const&) = delete;
    void operator=(Builder const&) = delete;

    /// The root chunk of the RIFF.
    ListChunkBuilder* _rootChunk = nullptr;

    /// Arena to use for all allocations.
    MemoryArena _arena;
};

/// A stateful cursor for a RIFF::Builder.
///
/// Represents a kind of pointer to a location in
/// the hierarchy of RIFF chunks, and allows for
/// new chunks to be added at that location.
///
struct BuildCursor
{
public:
    /// Construct a cursor writing into no chunk.
    BuildCursor();

    /// Construct a cursor writing into the given `chunk`.
    BuildCursor(ChunkBuilder* chunk);

    /// Construct a cursor writing at the root of the given `builder`.
    ///
    /// Note that this is not the same as constructing a
    /// cursor for the root chunk of `builder`. Instead, adding
    /// a chunk via this cursor will add/create the root chunk
    /// of the entire RIFF hierarchy.
    ///
    BuildCursor(Builder& builder);

    /// Get the RIFF being written into, if any.
    RIFF::Builder* getRIFFBuilder() const { return _riffBuilder; }

    /// Get the current chunk being written into, if any.
    ChunkBuilder* getCurrentChunk() const { return _currentChunk; }

    /// Set the current chunk to write into.
    void setCurrentChunk(ChunkBuilder* chunk);

    /// Append a new data chunk to the current list chunk.
    DataChunkBuilder* addDataChunk(Chunk::Type type);

    /// Append a complete data chunk to the current list chunk.
    void addDataChunk(Chunk::Type type, void const* data, size_t size);

    /// Append a new data chunk to the current list chunk.
    ListChunkBuilder* addListChunk(Chunk::Type type);

    /// Begin a new data chunk as a child of the current list chunk.
    ///
    /// On return, the cursor will be set to write into the new chunk.
    ///
    void beginDataChunk(Chunk::Type type);

    /// Begin a new list chunk as a child of the current list chunk.
    ///
    /// On return, the cursor will be set to write into the new chunk.
    ///
    void beginListChunk(Chunk::Type type);

    /// End the current chunk.
    ///
    /// Sets the cursor to write to the parent of the chunk that was ended.
    ///
    void endChunk();

    /// Append data onto the current data chunk.
    void addData(void const* data, Size size);

    /// Write data onto the current data chunk.
    template<typename T>
    void addData(T const& value)
    {
        addData(&value, sizeof(value));
    }

    /// Append existing data to the current data chunk.
    ///
    /// The caller takes responsibility for ensuring that
    /// the passed-in data pointer will remain valid for
    /// the rest of the lifetime of the enclosing RIFF
    /// builder.
    ///
    void addUnownedData(void const* data, Size size);

    /// Base type for RAII helpers to pair begin/end chunk calls.
    struct ScopedChunk
    {
    protected:
        ScopedChunk(BuildCursor& cursor)
            : _cursor(cursor)
        {
        }

        ~ScopedChunk() { _cursor.endChunk(); }

    private:
        BuildCursor& _cursor;
    };

    struct ScopedDataChunk : ScopedChunk
    {
    public:
        ScopedDataChunk(BuildCursor& cursor, Chunk::Type type)
            : ScopedChunk(cursor)
        {
            cursor.beginDataChunk(type);
        }
    };

    struct ScopedListChunk : ScopedChunk
    {
    public:
        ScopedListChunk(BuildCursor& cursor, Chunk::Type type)
            : ScopedChunk(cursor)
        {
            cursor.beginListChunk(type);
        }
    };

private:
    RIFF::Builder* _riffBuilder = nullptr;
    ChunkBuilder* _currentChunk = nullptr;
};

#define SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(CURSOR, TYPE)    \
    ::Slang::RIFF::BuildCursor::ScopedDataChunk SLANG_CONCAT( \
        _scopedRIFFBuilderDataChunk,                          \
        __LINE__)(CURSOR, TYPE)

#define SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(CURSOR, TYPE)    \
    ::Slang::RIFF::BuildCursor::ScopedListChunk SLANG_CONCAT( \
        _scopedRIFFBuilderListChunk,                          \
        __LINE__)(CURSOR, TYPE)

} // namespace RIFF

/// A simple helper for reading from a blob.
///
struct MemoryReader
{
    //
    // TODO: This type should eventually either find
    // a home somewhere that has nothing to do with
    // RIFF files, or its usage in RIFF-related contexts
    // should be replaced with other types.
    //

public:
    /// Initialize a reader with no bytes remaining.
    ///
    MemoryReader() {}

    /// Initialize a reader for the given blob.
    MemoryReader(void const* data, Size size)
        : _cursor(static_cast<Byte const*>(data)), _remainingSize(size)
    {
    }

    /// Read data into the given buffer.
    ///
    /// Fails if `size` is greater than the
    /// amount of data remaining.
    ///
    SlangResult read(void* dst, Size size)
    {
        if (size > getRemainingSize())
        {
            return SLANG_FAIL;
        }
        ::memcpy(dst, _cursor, size);
        _cursor += size;
        _remainingSize -= size;
        return SLANG_OK;
    }

    /// Read data into the given value.
    ///
    /// Fails if `sizeof(dst)` is greater than the
    /// amount of data remaining.
    ///
    template<typename T>
    SlangResult read(T& dst)
    {
        return read(&dst, sizeof(dst));
    }

    /// Skip over the given number of bytes.
    ///
    /// Fails if `size` is greater than the
    /// amount of data remaining.
    ///
    SlangResult skip(Size size)
    {
        if (size > getRemainingSize())
        {
            return SLANG_FAIL;
        }
        _cursor += size;
        _remainingSize -= size;
        return SLANG_OK;
    }

    /// Get a pointer to the data that remains to be read.
    Byte const* getRemainingData() const { return _cursor; }

    /// Get the size of the data that remains to be read.
    Size getRemainingSize() const { return _remainingSize; }

private:
    Byte const* _cursor = nullptr;
    Size _remainingSize = 0;
};

} // namespace Slang

#endif
