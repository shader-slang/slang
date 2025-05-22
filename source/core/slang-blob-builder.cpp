// slang-blob-builder.cpp
#include "slang-blob-builder.h"

namespace Slang
{

//
// ChipBuilder
//

ShardBuilder::ShardBuilder(Kind kind)
    : _kind(kind)
{
}

void ShardBuilder::_writeTo(Stream* stream, Size inSelfOffset)
{
    switch (_kind)
    {
    case Kind::Data:
        {
            stream->write(_data.ptr, _size);
        }
        break;

    case Kind::RelativePtr:
        {
            auto targetChunk = _relativePtr.targetChunk;
            SLANG_ASSERT(targetChunk);

            auto selfOffset = intptr_t(inSelfOffset);
            auto targetOffset = intptr_t(targetChunk->_getCachedOffset());

            intptr_t relativeOffset = targetOffset - selfOffset;

            switch (_size)
            {
            case sizeof(Int32):
                {
                    auto value = Int32(relativeOffset);
                    stream->write(&value, sizeof(value));
                }
                break;

            case sizeof(Int64):
                {
                    auto value = Int64(relativeOffset);
                    stream->write(&value, sizeof(value));
                }
                break;

            default:
                SLANG_UNEXPECTED("unsupported relative pointer size");
                break;
            }
        }
        break;

    default:
        SLANG_UNEXPECTED("unknown Fossil::ShardBuilder::Kind");
        break;
    }
}

//
// ChunkBuilder
//

void ChunkBuilder::setAlignmentToAtLeast(Size alignment)
{
    SLANG_ASSERT(alignment > 0);
    SLANG_ASSERT(isPowerOfTwo(alignment));

    _contentAlignment = std::max(_contentAlignment, alignment);
}

void ChunkBuilder::writePaddingToAlignTo(Size alignment)
{
    setAlignmentToAtLeast(alignment);

    auto alignedSize = roundUpToAlignment(_contentSize, alignment);

    auto requiredPaddingSize = alignedSize - _contentSize;

    while (requiredPaddingSize)
    {
        Byte padByte = 0;
        writeData(&padByte, sizeof(padByte));
        requiredPaddingSize -= sizeof(padByte);
    }
}

ChunkBuilder::ChunkBuilder(SlabBuilder* parentSlab)
    : _parentSlab(parentSlab)
{
}

Size ChunkBuilder::getContentSize() const
{
    return _contentSize;
}

Size ChunkBuilder::getPrefixSize() const
{
    if (!_prefixShard)
        return 0;

    return _prefixShard->getSize();
}

Size ChunkBuilder::getAlignment() const
{
    return _contentAlignment;
}


void ChunkBuilder::writeData(void const* data, Size size)
{
    if (size == 0)
        return;

    // TODO: check if current chip can be extended to support the data...

    auto shard = _createDataShard(data, size);
    _childShards.add(shard);
    _contentSize += size;
}

void ChunkBuilder::addContentsOf(ChunkBuilder* otherChunk)
{
    //    auto otherPrefixShard = otherChunk->_prefixShard;
    auto otherChunkSize = otherChunk->getContentSize();
    auto otherChunkAlignment = otherChunk->getAlignment();
    auto otherChunkShards = otherChunk->_childShards;

    //    otherChunk->_prefixShard = nullptr;
    otherChunk->_contentSize = 0;
    otherChunk->_contentAlignment = 1;
    otherChunk->_childShards = InternallyLinkedList<ShardBuilder>();

    /*
    if (otherPrefixShard)
    {
        SLANG_ASSERT(!_prefixShard);
        SLANG_ASSERT(!_childShards.getFirst());
        _prefixShard = otherPrefixShard;
    }
    */

    writePaddingToAlignTo(otherChunkAlignment);
    _childShards.append(otherChunkShards);
    _contentSize += otherChunkSize;
}


ChunkBuilder* ChunkBuilder::addChunkAfter()
{
    return getParentSlab()->addChunkAfter(this);
}


void ChunkBuilder::_writeRelativePtr(ChunkBuilder* targetChunk, Size ptrSize)
{
    SLANG_ASSERT(ptrSize != 0);
    SLANG_ASSERT(ptrSize <= sizeof(UInt64));

    writePaddingToAlignTo(ptrSize);

    if (!targetChunk)
    {
        UInt64 value = 0;
        writeData(&value, ptrSize);
        return;
    }

    auto shard = _createRelativePtrShard(targetChunk, ptrSize);
    _childShards.add(shard);
    _contentSize += ptrSize;
}

void ChunkBuilder::_addPrefixRelativePtr(ChunkBuilder* targetChunk, Size ptrSize)
{
    SLANG_ASSERT(targetChunk != nullptr);
    SLANG_ASSERT(ptrSize != 0);

    SLANG_ASSERT(!_prefixShard);

    setAlignmentToAtLeast(ptrSize);

    auto shard = _createRelativePtrShard(targetChunk, ptrSize);
    _prefixShard = shard;
}

void ChunkBuilder::addPrefixData(void const* data, Size size)
{
    SLANG_ASSERT(data != nullptr);
    SLANG_ASSERT(size != 0);

    SLANG_ASSERT(!_prefixShard);

    setAlignmentToAtLeast(size);

    auto shard = _createDataShard(data, size);
    _prefixShard = shard;
}

ShardBuilder* ChunkBuilder::_createDataShard(void const* data, Size size)
{
    auto& arena = getParentSlab()->_getArena();
    auto shard = new (arena) ShardBuilder(ShardBuilder::Kind::Data);

    auto shardData = arena.allocateUnaligned(size);
    ::memcpy(shardData, data, size);

    shard->_data.ptr = shardData;
    shard->_size = size;

    return shard;
}

ShardBuilder* ChunkBuilder::_createRelativePtrShard(ChunkBuilder* targetChunk, Size ptrSize)
{
    auto& arena = getParentSlab()->_getArena();
    auto shard = new (arena) ShardBuilder(ShardBuilder::Kind::RelativePtr);

    shard->_relativePtr.targetChunk = targetChunk;
    shard->_size = ptrSize;

    return shard;
}

void ChunkBuilder::_writeTo(Stream* stream)
{
    auto chunkOffset = _getCachedOffset();
    if (_prefixShard)
    {
        auto prefixSize = _prefixShard->getSize();
        _prefixShard->_writeTo(stream, chunkOffset - prefixSize);
    }

    auto shardOffset = chunkOffset;
    for (auto shard : _childShards)
    {
        shard->_writeTo(stream, shardOffset);
        shardOffset += shard->getSize();
    }
    SLANG_ASSERT(shardOffset == chunkOffset + getContentSize());
}

//
// SlabBuilder
//

SlabBuilder::SlabBuilder()
    : _arena(4096)
{
}

void SlabBuilder::writeTo(Stream* stream)
{
    // First we scan through all the chunks to set their
    // absolute offsets, which enables us to compute the
    // correct values for relative pointers when we write
    // them out.
    //
    SLANG_MAYBE_UNUSED
    Size sizeComputed = _calcSizeAndSetCachedChunkOffsets();

    // Now we can scan through the chunks again and write
    // the bytes of each of their shards.
    //
    SLANG_MAYBE_UNUSED
    Size sizeWritten = _writeChunksTo(stream);

    SLANG_ASSERT(sizeComputed == sizeWritten);
}

Size SlabBuilder::_calcSizeAndSetCachedChunkOffsets()
{
    Size totalSize = 0;
    for (auto chunk : _chunks)
    {
        auto chunkPrefixSize = chunk->getPrefixSize();
        auto chunkContentSize = chunk->getContentSize();
        auto chunkAlignment = chunk->getAlignment();

        totalSize += chunkPrefixSize;
        auto chunkOffset = roundUpToAlignment(totalSize, chunkAlignment);

        chunk->_setCachedOffset(chunkOffset);

        totalSize = chunkOffset + chunkContentSize;
    }
    return totalSize;
}

Size SlabBuilder::_writeChunksTo(Stream* stream)
{
    Size totalSize = 0;
    for (auto chunk : _chunks)
    {
        auto chunkPrefixSize = chunk->getPrefixSize();
        auto chunkContentSize = chunk->getContentSize();
        auto chunkOffset = chunk->_getCachedOffset();

        SLANG_ASSERT(
            chunkOffset == roundUpToAlignment(totalSize + chunkPrefixSize, chunk->getAlignment()));

        auto paddingSize = chunkOffset - totalSize;

        SLANG_ASSERT(paddingSize >= chunkPrefixSize);
        paddingSize -= chunkPrefixSize;

        while (paddingSize--)
        {
            Byte padding = 0;
            stream->write(&padding, sizeof(padding));
        }

        chunk->_writeTo(stream);

        totalSize = chunkOffset + chunkContentSize;
    }

    return totalSize;
}

void SlabBuilder::writeToBlob(ISlangBlob** outBlob)
{
    OwnedMemoryStream stream(FileAccess::Write);
    writeTo(&stream);

    List<uint8_t> data;
    stream.swapContents(data);

    *outBlob = ListBlob::moveCreate(data).detach();
}

ChunkBuilder* SlabBuilder::createUnparentedChunk()
{
    auto chunk = new (_arena) ChunkBuilder(this);
    return chunk;
}

void SlabBuilder::addChunk(ChunkBuilder* chunk)
{
    // TODO: assert that it wasn't already added!

    _chunks.add(chunk);
}


ChunkBuilder* SlabBuilder::addChunk()
{
    auto chunk = new (_arena) ChunkBuilder(this);
    _chunks.add(chunk);
    return chunk;
}

ChunkBuilder* SlabBuilder::addChunkAfter(ChunkBuilder* existingChunk)
{
    auto newChunk = new (_arena) ChunkBuilder(this);
    _chunks.insertAfter(existingChunk, newChunk);
    return newChunk;
}

} // namespace Slang
