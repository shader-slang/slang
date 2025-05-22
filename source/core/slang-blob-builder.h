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
struct SlabBuilder;

/// A shard is a unit of contiguously-allocated data that makes
/// up part of a chunk. Shards are *not* meant to be directly
/// manipulated by users.
///
class ShardBuilder : public InternallyLinkedList<ShardBuilder>::Node
{
public:
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

    Kind _kind = Kind::Data;
    union
    {
        struct
        {
            void const* ptr;
        } _data;
        struct
        {
            ChunkBuilder* targetChunk;
        } _relativePtr;
    };
    Size _size;
};

/// A chunk is a logically contiguous unit, such that data can
/// only ever be appended to it. As a result, offsets that are
/// relative to the start of a chunk are meaningful, and can be
/// used to encode relative offsets for pointers.
///
class ChunkBuilder : public InternallyLinkedList<ChunkBuilder>::Node
{
public:
    SlabBuilder* getParentSlab() const { return _parentSlab; }

    Size getAlignment() const;

    Size getContentSize() const;

    void setAlignmentToAtLeast(Size alignment);

    void writePaddingToAlignTo(Size alignment);

    void writeData(void const* data, Size size);

    template<typename T>
    void writeRelativePtr(ChunkBuilder* targetChunk)
    {
        _writeRelativePtr(targetChunk, sizeof(T));
    }

    void addContentsOf(ChunkBuilder* chunk);

    Size getPrefixSize() const;

    ChunkBuilder* addChunkAfter();

    template<typename T>
    void addPrefixRelativePtr(ChunkBuilder* targetChunk)
    {
        _addPrefixRelativePtr(targetChunk, sizeof(T));
    }

    void addPrefixData(void const* data, Size size);

private:
    ChunkBuilder() = delete;
    ChunkBuilder(ChunkBuilder const&) = delete;
    ChunkBuilder(ChunkBuilder&&) = delete;

    friend struct SlabBuilder;
    friend class ShardBuilder;

    ChunkBuilder(SlabBuilder* parentSlab);

    Size _contentSize = 0;
    Size _contentAlignment = 1;

    InternallyLinkedList<ShardBuilder> _childShards;
    SlabBuilder* _parentSlab = nullptr;

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

struct SlabBuilder
{
public:
    SlabBuilder();

    void writeTo(Stream* stream);
    void writeToBlob(ISlangBlob** outBlob);

    ChunkBuilder* addChunk();
    ChunkBuilder* addChunkAfter(ChunkBuilder* chunk);

    ChunkBuilder* createUnparentedChunk();

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


} // namespace Slang

#endif
