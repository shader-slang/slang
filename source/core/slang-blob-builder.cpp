// slang-blob-builder.cpp
#include "slang-blob-builder.h"

namespace Slang
{

//
// BlobBuilder
//

BlobBuilder::BlobBuilder()
    : _arena(4096)
{
}

void BlobBuilder::writeTo(Stream* stream)
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

Size BlobBuilder::_calcSizeAndSetCachedChunkOffsets()
{
    Size totalSize = 0;
    for (auto chunk : _chunks)
    {
        auto chunkPrefixSize = chunk->getPrefixSize();
        auto chunkContentSize = chunk->getContentSize();
        auto chunkAlignment = chunk->getAlignment();

        // We add the size of the chunk prefix (if any) *before*
        // aligning the current offset. Doing it this way
        // means that sometimes the prefix can fit "for free"
        // in space that would otherwise be padding.
        //
        // For example, if the current `totalSize` is 4, the
        // `chunkAlignment` is 16, and the `chunkPrefixSize` is 8,
        // then the sequence will be:
        //
        // * Add `totalSize += chunkPrefixSize`, resulting in a `totalSize`
        //   of 12.
        //
        // * Round the `totalSize` up to the `chunkAlignment`, to compute
        //   a `chunkOffset` of 16.
        //
        // The result is that the chunk's content starts at offset 16,
        // and the prefix can occupy the 8 bytes before that (so the
        // prefix is at offset 8).
        //
        // In the best case this approach can save a few bytes here or
        // there when `chunkAlignment` is larger than the `chunkPrefixSize`,
        // and in the worst case it does no harm.

        totalSize += chunkPrefixSize;
        auto chunkOffset = roundUpToAlignment(totalSize, chunkAlignment);

        chunk->_setCachedOffset(chunkOffset);

        totalSize = chunkOffset + chunkContentSize;
    }
    return totalSize;
}

Size BlobBuilder::_writeChunksTo(Stream* stream)
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

        // The "padding" before the chunk's content also
        // includes the space reserved for the chunk's prefix.
        //
        // We can thus subtract the prefix size from the number
        // of pad bytes to write.

        SLANG_ASSERT(paddingSize >= chunkPrefixSize);
        paddingSize -= chunkPrefixSize;

        while (paddingSize--)
        {
            Byte padding = 0;
            stream->write(&padding, sizeof(padding));
        }

        // The `ChunkBuilder::_writeTo()` call will write the
        // prefix *and* the chunk content, so it is appropriate
        // to call it here even when the total number of bytes
        // written to the stream is not equal to the `chunkOffset`
        // (because in that case the total bytes written so far
        // should be `chunkOffset - chunkPrefixSize`).
        //
        chunk->_writeTo(stream);

        totalSize = chunkOffset + chunkContentSize;
    }

    return totalSize;
}

void BlobBuilder::writeToBlob(ISlangBlob** outBlob)
{
    OwnedMemoryStream stream(FileAccess::Write);
    writeTo(&stream);

    List<uint8_t> data;
    stream.swapContents(data);

    *outBlob = ListBlob::moveCreate(data).detach();
}

ChunkBuilder* BlobBuilder::createUnparentedChunk()
{
    auto chunk = new (_arena) ChunkBuilder(this);
    return chunk;
}

void BlobBuilder::addChunk(ChunkBuilder* chunk)
{
    // TODO(tfoley): it would be good to have a way to assert
    // that the chunk has not already been added.

    _chunks.add(chunk);
}

ChunkBuilder* BlobBuilder::addChunk()
{
    auto chunk = new (_arena) ChunkBuilder(this);
    _chunks.add(chunk);
    return chunk;
}

ChunkBuilder* BlobBuilder::addChunkAfter(ChunkBuilder* existingChunk)
{
    auto newChunk = new (_arena) ChunkBuilder(this);
    _chunks.insertAfter(existingChunk, newChunk);
    return newChunk;
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

ChunkBuilder::ChunkBuilder(BlobBuilder* parentBlob)
    : _parentBlob(parentBlob)
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
    // Adding no data should be a no-op.
    //
    if (size == 0)
        return;


    // The most interesting implementation detail here
    // is that we will try to detect cases where we
    // can re-use an existing `ShardBuilder` by adding the data
    // to the end of that shard's allocation.
    //
    // This is only possible because of the way that
    // we are using a single `MemoryArena` to allocate
    // everything, which makes it possible that the
    // next address the arena would return for an allocation
    // of `size` bytes is the same as the ending address
    // of the payload for the last shard of this chunk.
    //
    auto& arena = getParentBlob()->_getArena();

    // We start by checking if this chunk already has
    // a last shard that we could consider appending to.
    //
    auto lastShard = _childShards.getLast();
    if (lastShard && lastShard->_kind == ShardBuilder::Kind::Data)
    {
        // If there is a last shard, then we can compute
        // the end address of its payload, and see if
        // it is the same as the cursor of the arena
        // we are allocating from.
        //
        auto payload = lastShard->_data.ptr;
        auto payloadSize = lastShard->_size;
        auto payloadEnd = (Byte*)payload + payloadSize;
        if (payloadEnd == arena.getCursor())
        {
            // Now that we've confirmed that the shard's
            // payload ends at an address the arena could
            // conceivably allocate from, we need to ask
            // the arena to allocate `size` bytes from
            // the current block it is using, and see if
            // doing so succeeds.
            //
            if (arena.allocateCurrentUnaligned(size))
            {
                // At this point, we've confirmed that we
                // are in our special case, and the relevant
                // bytes have been allocated from the arena.
                //
                // Now we can simply write the new data at
                // what used to be the end address for the
                // shard's payload, and adjust its state
                // to account for the new allocation.
                //
                ::memcpy(payloadEnd, data, size);

                lastShard->_size = payloadSize + size;
                return;
            }
        }
    }

    // If the special case above doesn't apply, we simply
    // allocate a new shard to hold the data that was
    // passed in.
    //
    auto shard = _createDataShard(data, size);
    _childShards.add(shard);
    _contentSize += size;
}

void ChunkBuilder::addContentsOf(ChunkBuilder* otherChunk)
{
    auto otherPrefixShard = otherChunk->_prefixShard;
    auto otherChunkSize = otherChunk->getContentSize();
    auto otherChunkAlignment = otherChunk->getAlignment();
    auto otherChunkShards = otherChunk->_childShards;

    otherChunk->_prefixShard = nullptr;
    otherChunk->_contentSize = 0;
    otherChunk->_contentAlignment = 1;
    otherChunk->_childShards = InternallyLinkedList<ShardBuilder>();

    if (otherPrefixShard)
    {
        // If the other chunk included a prefix, then
        // it only makes sense to append it in the case
        // where *this* chunk is completely empty.

        SLANG_ASSERT(!_prefixShard);
        SLANG_ASSERT(!_childShards.getFirst());
        _prefixShard = otherPrefixShard;
    }

    writePaddingToAlignTo(otherChunkAlignment);
    _childShards.append(otherChunkShards);
    _contentSize += otherChunkSize;
}

ChunkBuilder* ChunkBuilder::addChunkAfter()
{
    return getParentBlob()->addChunkAfter(this);
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
    auto& arena = getParentBlob()->_getArena();
    auto shard = new (arena) ShardBuilder(ShardBuilder::Kind::Data);

    auto shardData = arena.allocateUnaligned(size);
    ::memcpy(shardData, data, size);

    shard->_data.ptr = shardData;
    shard->_size = size;

    return shard;
}

ShardBuilder* ChunkBuilder::_createRelativePtrShard(ChunkBuilder* targetChunk, Size ptrSize)
{
    auto& arena = getParentBlob()->_getArena();
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
        // Note that the prefix is written *before* the
        // starting offset of the chunk's content, so we
        // compute the appropriate offset to pass down.
        //
        auto prefixSize = _prefixShard->getSize();
        auto prefixOffset = chunkOffset - prefixSize;

        _prefixShard->_writeTo(stream, prefixOffset);
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
// ShardBuilder
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

} // namespace Slang
