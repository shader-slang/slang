// slang-blob-builder.h
#ifndef SLANG_BLOB_BUILDER_H
#define SLANG_BLOB_BUILDER_H

// This file provides utilities for building "blobs" of data
// where, for purposes, a blob is a contiguous sequence of
// bytes where the interpretation *of* those bytes depends
// only on the bytes themselves, and not other factors like
// the in-memory address of the blob, or the address/contents
// of memory not in the blob.
//
// Superficially, the task seems simple: just maintain a
// dynamically-sized array of bytes and append to it until
// you're done. If that's what you need, you're probably
// better off just using an `OwnedMemoryStream`.
//
// The utilities in this file deal with the case where you
// want to build some kind of offset-based data structure,
// so that parts of the blob will store byte offsets to
// other parts, while also being able to build parts of
// that structure "out of order", so that the final offset
// of a particular piece of data in the blob may not be
// known until everything *before* it has been fully built.

#include "slang-basic.h"
#include "slang-internally-linked-list.h"
#include "slang-io.h"
#include "slang-memory-arena.h"

namespace Slang
{

inline constexpr bool isPowerOfTwo(Size value)
{
    return value > 0 && (value - 1 & value) == 0;
}

inline constexpr Size roundUpToAlignment(Size size, Size alignment)
{
    SLANG_ASSERT(isPowerOfTwo(alignment));

    auto alignmentMask = Size(alignment) - 1;
    return (size + alignmentMask) & ~alignmentMask;
}

class ShardBuilder;
class ChunkBuilder;
struct BlobBuilder;


/// A utility type for composing a binary blob.
///
/// A blob builder allows a blob to be composed as a sequence of discrete
/// chunks, allowing chunks to be added and written to in any order.
///
/// Chunks can contain (relative) pointers to one another, with the correct
/// relative offsets being computed as part of writing the entire blob out.
/// 
struct BlobBuilder
{
public:
    /// Construct an empty blob builder.
    BlobBuilder();

    /// Write the contents of the blob to the given `stream`.
    void writeTo(Stream* stream);

    /// Create a copy of the contents of the blob and assign to `outBlob`.
    void writeToBlob(ISlangBlob** outBlob);

    /// Add a new empty chunk to the end of the blob.
    ChunkBuilder* addChunk();

    /// Add a new empty chunk after the given `chunk`.
    ChunkBuilder* addChunkAfter(ChunkBuilder* chunk);

    /// Create a chunk that is not initially part of the blob.
    ///
    /// The contents of the returned chunk will only become
    /// part of the full blob if `addChunk()` is called later,
    /// *or* if the contents of the new chunk are moved into
    /// another chunk that gets added.
    ///
    ChunkBuilder* createUnparentedChunk();

    /// Add a chunk to the blob that was initially not part of the blob.
    void addChunk(ChunkBuilder* chunk);

private:
    InternallyLinkedList<ChunkBuilder> _chunks;
    MemoryArena _arena;

    friend class ChunkBuilder;
    friend class ShardBuilder;
    MemoryArena& _getArena() { return _arena; }

    Size _calcSizeAndSetCachedChunkOffsets();
    Size _writeChunksTo(Stream* stream);
};

/// A chunk is a logically contiguous unit, such that data can
/// only ever be appended to it. As a result, offsets that are
/// relative to the start of a chunk are meaningful, and can be
/// used to encode relative offsets for pointers.
///
/// Every `ChunkBuilder` is owned by its parent `BlobBuilder`.
/// A pointer to a `ChunkBuilder` will only be valid during the
/// lifetime of the parent `BlobBuilder`.
///
/// Conceptually, a `ChunkBuilder` has the following:
/// 
/// * A minimum *alignment* in bytes (initially 1)
/// 
/// * Zero or more bytes of *content* data (initially empty)
/// 
/// * An optional *prefix* consisting of zero or more bytes (initially absent)
///
/// The content may be a mix of raw data (e.g., added via `writeData()`),
/// relative pointers to other chunks (`writeRelativePtr()`) and padding
/// bytes (`writePaddingToAlignTo()`).
///
/// The prefix may only be either a single relative pointer, or a
/// single range of raw data bytes.
///
/// When the parent blob written out as a flat buffer of bytes, the
/// following are guaranteed:
/// 
/// * The content of the chunk will be a contiguous range of bytes
///   starting at some offset, and will not overlap any other chunks.
/// 
/// * The byte offset where the chunk contents start will be a multiple
///   of the chunk's minimum alignment.
/// 
/// * The bytes of the prefix (if any) will immediately precede the
///   bytes of the content.
///
class ChunkBuilder : public InternallyLinkedList<ChunkBuilder>::Node
{
public:
    /// Get the blob builder that this chunk belongs to.
    BlobBuilder* getParentBlob() const { return _parentBlob; }

    /// Get the required alignment of this chunk.
    ///
    /// The minimum alignment for a chunk starts at 1,
    /// and may be increased by operations such as
    /// `setAlignmentToAtLeast()` and `writePaddingToAlinTo()`.
    ///
    Size getAlignment() const;

    /// Potentially increases the required alignment of this chunk.
    ///
    /// If the alignment of the chunk is less than `alignment`,
    /// then it will be increased to `alignment`.
    ///
    /// The `alignment` passed in must be a power of two.
    ///
    void setAlignmentToAtLeast(Size alignment);

    /// Get the size in bytes of the content of this chunk.
    ///
    /// Note that the size is not necessarily a multiple of the
    /// alignment of the chunk.
    /// 
    Size getContentSize() const;

    /// Write data into the chunk.
    ///
    /// The chunk will retain a copy of the data passed in,
    /// so the `data` pointer only needs to be valid for
    /// the duration of the call.
    ///
    /// Note that this operation does *not* adjust the
    /// alignment of the chunk in any way.
    ///
    /// The data must only contain types that can be copied
    /// bit-for-bit, and that do not depend on addresses
    /// in meory. In particular no pointers (absolute or
    /// relative) should be written.
    /// 
    void writeData(void const* data, Size size);

    /// Append padding bytes to this chunk until its content size
    /// is a multiple of `alignment`.
    ///
    /// May also increase the alignment of the chunk, as if
    /// calling `setAlignmentToAtLeast(alignment)`.
    /// 
    /// The padding bytes will all be zero.
    ///
    void writePaddingToAlignTo(Size alignment);

    /// Write a relative pointer to the given `targetChunk`.
    ///
    /// The type parameter `T` is used to determine the size
    /// of the relative pointer (should be either 4 or 8 bytes).
    ///
    /// The bytes that eventually get written will contain
    /// the computed offset of `targetChunk` minus the computed
    /// offset of the first byte of the relative pointer itself.
    /// 
    /// Acts as is `writePaddingToAlignTo(sizeof(T))` were
    /// called immediately before.
    ///
    template<typename T>
    void writeRelativePtr(ChunkBuilder* targetChunk)
    {
        _writeRelativePtr(targetChunk, sizeof(T));
    }

    /// Append the contents of another chunk to this one.
    ///
    /// This *moves* all of the contents of `chunk` into `this`.
    /// After the operation completes `chunk` will be an empty
    /// chunk with one-byte alignment.
    ///
    /// This operation is useful when accumulating data that
    /// needs to be appended to the chunk, but where the correct
    /// alignment for that data is not yet known; a use can
    /// effectively create a temporary "sub-chunk" and then append
    /// it to the main chunk once its correct alignment is known.
    ///
    void addContentsOf(ChunkBuilder* chunk);

    /// Get the size in bytes of the prefix of this chunk, if any.
    ///
    /// The prefix will be written so that the *end* offset of the
    /// prefix data is the same as the starting offset of the
    /// chunk's content.
    ///
    Size getPrefixSize() const;

    /// Add a prefix to this chunk, consisting of raw data.
    ///
    /// This chunk must not already have a prefix.
    ///
    void addPrefixData(void const* data, Size size);

    /// Add a prefix to this chunk, consisting of a relative pointer.
    ///
    /// This chunk must not already have a prefix.
    ///
    /// Updates the alignment of the chunk as if making a call to
    /// `setAlignmentToAtLeast(sizeof(T))`.
    /// 
    template<typename T>
    void addPrefixRelativePtr(ChunkBuilder* targetChunk)
    {
        _addPrefixRelativePtr(targetChunk, sizeof(T));
    }

    /// Insert a new chunk into the blob, immediately after this chunk.
    ///
    ChunkBuilder* addChunkAfter();

private:
    ChunkBuilder() = delete;
    ChunkBuilder(ChunkBuilder const&) = delete;
    ChunkBuilder(ChunkBuilder&&) = delete;

    friend struct BlobBuilder;
    friend class ShardBuilder;

    ChunkBuilder(BlobBuilder* parentBlob);

    Size _contentSize = 0;
    Size _contentAlignment = 1;

    InternallyLinkedList<ShardBuilder> _childShards;
    BlobBuilder* _parentBlob = nullptr;

    Size _cachedOffset = ~Size(0);
    Size _getCachedOffset() { return _cachedOffset; }
    void _setCachedOffset(Size offset) { _cachedOffset = offset; }

    void _writeRelativePtr(ChunkBuilder* targetChunk, Size ptrSize);

    ShardBuilder* _createDataShard(void const* data, Size size);
    ShardBuilder* _createRelativePtrShard(ChunkBuilder* targetChunk, Size ptrSize);

    ShardBuilder* _prefixShard = nullptr;

    void _writeTo(Stream* stream);

    void _addPrefixRelativePtr(ChunkBuilder* targetChunk, Size ptrSize);
};

/// A shard is a unit of contiguously-allocated data that makes
/// up part of a chunk.
/// 
/// Shards are *not* meant to be directly manipulated by users;
/// they are an implementation detail of `ChunkBuilder`.
///
/// Every `ShardBuilder` is owned by its parent `ChunkBuilder`.
/// A pointer to a `ShardBuilder` will only be valid during the
/// lifetime of the parent `ChunkBuilder`.
///
class ShardBuilder : public InternallyLinkedList<ShardBuilder>::Node
{
public:

    // There are two kinds of shards that may appear in a chunk:
    //
    // * Shards that hold plain data that will be part of the
    //   serialized chunk.
    //
    // * Shards that represent a relative pointer to some chunk,
    //   which cannot have their exact binary value determined
    //   until the offsets of chunk/shards have been finalized.

    enum class Kind
    {
        Data,
        RelativePtr,
    };

    Size getSize() const { return _size; }

private:
    ShardBuilder() = delete;
    ShardBuilder(ShardBuilder const&) = delete;
    ShardBuilder(ShardBuilder&&) = delete;

    friend class ChunkBuilder;
    ShardBuilder(Kind kind);

    void _writeTo(Stream* stream, Size selfOffset);

    /// Kind of this shard (data or relative pointer)
    Kind _kind = Kind::Data;

    union
    {
        /// Used when `_kind == Kind::Data`
        struct
        {
            void const* ptr;
        } _data;

        /// Used when `_kind == Kind::RelativePtr`
        struct
        {
            ChunkBuilder* targetChunk;
        } _relativePtr;
    };

    // Size of this shard in bytes.
    Size _size = 0;
};

} // namespace Slang

#endif
