// slang-riff.cpp
#include "slang-riff.h"

#include "slang-blob.h"
#include "slang-com-helper.h"

namespace Slang
{
namespace RIFF
{

Size _roundUpToChunkAlignment(Size size)
{
    auto alignmentMask = Size(Chunk::kChunkAlignment) - 1;
    return (size + alignmentMask) & ~alignmentMask;
}

//
// RIFF::Chunk
//

//
// RIFF::DataChunk
//

void DataChunk::writePayloadInto(void* outData, Size size) const
{
    SLANG_ASSERT(size <= getPayloadSize());
    ::memcpy(outData, getPayload(), size);
}

//
// RIFF::BoundsCheckedChunkPtr
//

void BoundsCheckedChunkPtr::_set(Chunk const* chunk, Size sizeLimit)
{
    // We start by clearing out the state of this
    // pointer, so that we can early-out if any
    // validation checks fail, and be sure we
    // have a null pointer.
    //
    _ptr = nullptr;
    _sizeLimit = 0;

    // If there's nothing to point to, then the pointer
    // should be null anyway.
    //
    if (!chunk || !sizeLimit)
    {
        return;
    }

    // Because this type can be used to traverse RIFF
    // chunks that were loaded into memory from in-theory
    // untrusted sources, we try to provide some validation
    // checks to make sure that access to the chunk will
    // be safe (or as safe as we can easily ensure).

    // If the available size isn't even enough for the
    // header of a RIFF chunk, then something is wrong.
    //
    if (sizeLimit < sizeof(Chunk::Header))
    {
        SLANG_UNEXPECTED("invalid RIFF");
        return;
    }

    // Once we've checked that there is enough space
    // for a valid RIFF header, we can read the
    // size that the `chunk` reports itself as having.
    //
    auto reportedSize = chunk->getTotalSize();

    // If the reported size is too small, then something
    // is wrong.
    //
    if (reportedSize < sizeof(Chunk::Header))
    {
        SLANG_UNEXPECTED("invalid RIFF");
        return;
    }

    // If the reported size is bigger than the size limit,
    // then it must be invalid (it is reporting itself as
    // bigger than the region of memory that is supposed
    // to contain it).
    //
    if (reportedSize > sizeLimit)
    {
        SLANG_UNEXPECTED("invalid RIFF");
        return;
    }

    // If the chunk claims to be a list chunk, then it
    // must be big enough to hold the larger header
    // that list chunks use.
    //
    if (as<ListChunk>(chunk))
    {
        if (reportedSize < sizeof(ListChunk::Header))
        {
            SLANG_UNEXPECTED("invalid RIFF");
            return;
        }
    }

    // At this point we've performed some basic validation
    // telling us that the chunk header appears plausible.
    // This does not mean that we've fully validated the
    // hierarchy of child chunks under it (in the case of
    // a list chunk), but that validation can be performed
    // on-demand while descending the hierarchy.

    _ptr = chunk;
    _sizeLimit = sizeLimit;
}

void BoundsCheckedChunkPtr::_set(Chunk const* chunk)
{
    // In the case where we are being set to point to a
    // single chunk, we have to assume that whatever
    // code derived the `chunk` pointer has validated
    // that it is safe to access its header.
    //
    // We will simply set up a pointer that can reference
    // the `chunk` itself, as well as any of its children
    // (if it has any), but that cannot be used to access
    // further sibling chunks under the same parent.
    //
    _set(chunk, chunk->getTotalSize());
}

BoundsCheckedChunkPtr BoundsCheckedChunkPtr::getNextSibling() const
{
    SLANG_ASSERT(_ptr != nullptr);
    if (!_ptr)
        return nullptr;

    // The RIFF chunk reports its own size, and when navigating
    // the children of a list chunk, each child chunk starts
    // at the next (aligned) offset after the previous one.
    //
    auto chunkSize = _ptr->getTotalSize();

    // As a simple validation check, we check for a chunk that
    // reports its size as something bigger than the available
    // size; that would represent an invalid input.
    //
    if (chunkSize > _sizeLimit)
    {
        SLANG_UNEXPECTED("invalid RIFF chunk size");
        UNREACHABLE_RETURN(nullptr);
    }

    // The next chunk (if there is one) would start at the
    // next offset after this chunk, rounded up to the minimum
    // alignment for a chunk. Thus, we round up the reported
    // size of this chunk to compute the offset to the next
    // chunk.
    //
    auto offsetToNextChunk = _roundUpToChunkAlignment(chunkSize);

    // If stepping forward by the given number of bytes would
    // cause us to exceed our size limit, then we have reached
    // the end of the list of sibling chunks, and should
    // return a null pointer.
    //
    if (offsetToNextChunk >= _sizeLimit)
        return nullptr;

    auto nextChunk = (RIFF::Chunk const*)(offsetToNextChunk + (Byte const*)_ptr);
    auto nextSizeLimit = _sizeLimit - offsetToNextChunk;

    return BoundsCheckedChunkPtr(nextChunk, nextSizeLimit);
}


//
// RIFF::ListChunk
//

BoundsCheckedChunkPtr ListChunk::getFirstChild() const
{
    // Because this type could be used to navigate
    // an untrusted RIFF that has been loaded into memory,
    // we make some efforts to validate that things
    // seem okay as we navigate it.

    // The first child of a list chunk (if it has any)
    // comes right after the list header.
    //
    Size firstChildOffset = sizeof(ListChunk::Header);

    // The size that the parent chunk reports should
    // be appropriate to store the header.
    //
    // Note that in order to compute the reported size
    // we are *accessing* the header, so if there really
    // are too few bytes available, it is up to whatever
    // code computed this `ListChunk*` to have done
    // their own validation checks.
    //
    Size reportedParentSize = getTotalSize();
    if (reportedParentSize < firstChildOffset)
    {
        SLANG_UNEXPECTED("invalid RIFF");
        UNREACHABLE_RETURN(nullptr);
    }

    // The total size that the childen of this chunk can
    // consume is all of the reported size of the parent,
    // after the `ListChunk::Header`.
    //
    Size availableSizeForChildren = reportedParentSize - firstChildOffset;

    // The available size can be zero, in the case where
    // the parent chunk has no children.
    //
    if (availableSizeForChildren == 0)
        return nullptr;

    // There will be padding after the `ListChunk::Header` to align the first child to 8 bytes.
    // However, if the parent chunk has no children, then its total size will only be the size of
    // `ListChunk::Header` without the padding. Thus, we can only align the child offset after the
    // previous size checks.
    firstChildOffset = _roundUpToChunkAlignment(firstChildOffset);
    availableSizeForChildren = reportedParentSize - firstChildOffset;

    // If the parent chunk has a non-zero size, then it should
    // have at least one child, and the available size had better
    // be big enough to at least hold the *header* of that first
    // child.
    //
    if (availableSizeForChildren < sizeof(Chunk::Header))
    {
        SLANG_UNEXPECTED("invalid RIFF");
        UNREACHABLE_RETURN(nullptr);
    }

    // At this point we've convinced ourselves that there is
    // conceivably enough space for at least one child chunk,
    // so we will form a `BoundsCheckedChunkPtr` to it, which
    // will trigger further validity checks on that child chunk.
    //
    auto firstChild = (Chunk const*)(firstChildOffset + (Byte const*)this);
    return BoundsCheckedChunkPtr(firstChild, availableSizeForChildren);
}

DataChunk const* ListChunk::findDataChunk(Chunk::Type type) const
{
    for (auto chunk : getChildren())
    {
        auto dataChunk = as<DataChunk>(chunk);
        if (!dataChunk)
            continue;

        if (dataChunk->getType() != type)
            continue;

        return dataChunk;
    }
    return nullptr;
}

ListChunk const* ListChunk::findListChunk(Chunk::Type type) const
{
    for (auto chunk : getChildren())
    {
        auto listChunk = as<ListChunk>(chunk);
        if (!listChunk)
            continue;

        if (listChunk->getType() != type)
            continue;

        return listChunk;
    }
    return nullptr;
}

ListChunk const* ListChunk::findListChunkRec(Chunk::Type type) const
{
    // Note: The search being performed here could
    // be implemented without any need for recursion
    // (or a stack), by taking advantage of the way
    // that RIFF chunks are laid out. If we have some
    // chunk C, then the next aligned offset in memory
    // after C is either at the end of the hierarchy,
    // or it is the next sibling of one of C's ancestors
    // (where C is being counted as its own ancestor).
    //
    // However, it's not really clear if there's enough
    // of a benefit to justify that more subtle implementation.

    if (getType() == type)
        return this;

    for (auto chunk : getChildren())
    {
        auto listChunk = as<ListChunk>(chunk);
        if (!listChunk)
            continue;

        auto found = listChunk->findListChunkRec(type);
        if (!found)
            continue;

        return found;
    }
    return nullptr;
}


//
// RIFF::RootChunk
//

RootChunk const* RootChunk::getFromBlob(void const* data, size_t dataSize)
{
    // Our goal is to determine whether the given
    // blob superficially looks like a RIFF.

    // The data pointer should be non-null if there
    // was any data passed in.
    //
    SLANG_ASSERT(data || !dataSize);

    // If there's no data, then it's obvious not usable.
    //
    if (!data)
        return nullptr;

    // If there isn't even enough data to store the header
    // for a root, chunk, then the blob is too small.
    //
    if (dataSize < sizeof(RootChunk::Header))
        return nullptr;

    // We cast the data pointer to a root chunk here, so that
    // we can access the fields in the header, but we are not
    // yet convinced it is actually a valid RIFF, so we may
    // still return `nullptr`.
    //
    auto rootChunk = reinterpret_cast<RootChunk const*>(data);

    // The root chunk of a valid RIFF should have the `"RIFF"`
    // tag. This acts as a kind of "magic number" to mark the
    // start of a RIFF.
    //
    if (rootChunk->getTag() != RootChunk::kTag)
        return nullptr;

    // By reading the size field from the root chunk, we can
    // determine how big of a file the root chunk claims that
    // we have.
    //
    auto reportedSize = rootChunk->getTotalSize();

    // If the size implied by the RIFF header is larger than the
    // blob, then we do not have a properly structured RIFF,
    // and we would be at risk of reading past the end of the
    // buffer if we attempted to use it.
    //
    if (reportedSize > dataSize)
        return nullptr;

    // Note: It is possible that the `reportedSize` is strictly
    // *less than* the `dataSize` that was passed in, and there
    // is a policy choice to be made about how to handle that case.
    //
    // We err on the side of leniency here, because the client who
    // is calling this function might intentionally be storing
    // additional data in the same blob, after the RIFF, and could
    // use the RIFF's ability to report its own size as a way to
    // locate that data.

    // Note: At this point we could recursively walk the hierarchy
    // of the RIFF and validate that all the contained chunks appear
    // valid in terms of the sizes they report, but doing so would
    // take an amount of time that scales with the size of the RIFF,
    // and our goal here is to be efficient.
    //
    // Access to the data through the `RIFF::Chunk` API will do its
    // best to validate the information in chunks as they are
    // accessed. The code is attempting to be able to catch
    // corrupted or accidentally malformed input, but is not aspiring
    // to anything like proper security.

    return rootChunk;
}

RootChunk const* RootChunk::getFromBlob(ISlangBlob* blob)
{
    SLANG_ASSERT(blob);
    return getFromBlob(blob->getBufferPointer(), blob->getBufferSize());
}

//
// RIFF::ChunkBuilder
//

Size ChunkBuilder::_updateCachedTotalSize() const
{
    Size totalSize = 0;
    if (auto dataChunk = as<DataChunkBuilder>(this))
    {
        // Every chunk starts with a header.
        //
        totalSize += sizeof(DataChunk::Header);

        // After the header comes the payload of
        // the chunk, which for a data chunk
        // will be the concatenation of all its
        // shards.
        //
        for (auto shard : dataChunk->getShards())
        {
            totalSize += shard->getPayloadSize();
        }
    }
    else if (auto listChunk = as<ListChunkBuilder>(this))
    {
        // A list chunk starts with a header, just
        // like a data chunk, although its header
        // is larger.
        //
        totalSize += sizeof(ListChunk::Header);

        // After the header come the child chunks, in order.
        //
        for (auto child : listChunk->getChildren())
        {
            // We recursively poke the child chunks to
            // update their cached total size, so that
            // we can be sure we are getting correct
            // information.
            //
            auto childSize = child->_updateCachedTotalSize();

            // We cannot simply add the size of the child
            // chunk directly to `totalSize`, because a
            // RIFF guarantees that every chunk must start
            // on a suitably aligned boundary. Thus, we
            // first round `totalSize` up to the necessary
            // alignment, and then add the child's size.
            //
            totalSize = _roundUpToChunkAlignment(totalSize);
            totalSize += childSize;
        }
    }
    else
    {
        SLANG_UNREACHABLE("RIFF chunk must be data or list");
    }

    _cachedTotalSize = totalSize;
    return totalSize;
}

Result ChunkBuilder::_writeTo(Stream* stream) const
{
    // The size information that gets written into
    // the chunk header will be based on the cached
    // size information for this chunk.
    //
    // If nobody has called `_updateCachedTotalSize()`
    // at all, then that is a problem.
    //
    SLANG_ASSERT(_getCachedTotalSize() >= sizeof(Chunk::Header));

    // The size that gets written into the chunk header
    // is the total size of the chunk, ecluding the chunk
    // header. Note that this size will *include* the
    // additional field of the list chunk header.
    //
    Size totalSizeExcludingChunkHeader = _getCachedTotalSize() - sizeof(Chunk::Header);

    // Because the size field in the header is only 32 bits,
    // we want to double-check that it can actually represent
    // the size of the data to be written into it.
    //
    UInt32 sizeToWriteInHeader = UInt32(totalSizeExcludingChunkHeader);
    SLANG_ASSERT(Size(sizeToWriteInHeader) == totalSizeExcludingChunkHeader);

    if (auto dataChunk = as<DataChunkBuilder>(this))
    {
        // We start by writing the header.
        //
        DataChunk::Header header;
        header.size = sizeToWriteInHeader;

        // The tag of a data chunk is its type `FourCC`.
        //
        header.tag = dataChunk->getType();

        // Once we've filled in the header fields, we
        // can write it to the output stream.
        //
        SLANG_RETURN_ON_FAIL(stream->write(&header, sizeof(header)));

        // Now we can simply write the payload bytes,
        // which are the concatenation of the payloads
        // of all the shards.
        //
        for (auto shard : dataChunk->getShards())
        {
            auto payload = shard->getPayload();
            auto payloadSize = shard->getPayloadSize();
            SLANG_RETURN_ON_FAIL(stream->write(payload, payloadSize));
        }
    }
    else if (auto listChunk = as<ListChunkBuilder>(this))
    {
        // We start by writing the header.
        //
        ListChunk::Header header;
        header.chunkHeader.size = sizeToWriteInHeader;

        // The tag of a list chunk is either `"RIFF"`
        // (for a root chunk) or `"LIST"` (for any
        // other list chunk).
        //
        header.chunkHeader.tag =
            listChunk->getKind() == Chunk::Kind::Root ? RootChunk::kTag : ListChunk::kTag;

        // The type of a list chunk is stored in the
        // additional header field after the base
        // chunk header.
        //
        header.type = listChunk->getType();

        // Once we've filled in the header fields, we
        // can write it to the output stream.
        //
        SLANG_RETURN_ON_FAIL(stream->write(&header, sizeof(header)));

        // Now we recursively write each of the child chunks,
        // keeping track of the total size so far, so that
        // we can insert padding as needing to bring things
        // up to alignment.
        //
        Size totalSize = sizeof(header);
        for (auto child : listChunk->getChildren())
        {
            // We note the total size written so far,
            // as well as the size after rounding up
            // to the required alignment.
            //
            auto unalignedSize = totalSize;
            auto alignedSize = _roundUpToChunkAlignment(unalignedSize);
            SLANG_ASSERT(alignedSize >= unalignedSize);

            // If the aligned size is greater than the
            // unaligned size, then we may need to write
            // some padding bytes into the output stream.
            //
            auto paddingSize = alignedSize - unalignedSize;

            // The amount of padding to be inserted must
            // always be less than the minimum chunk alignment.
            //
            SLANG_ASSERT(paddingSize < Chunk::kChunkAlignment);

            // We'll write padding bytes to get things up
            // to the necessary alignment.
            //
            auto remainingPaddingToWrite = paddingSize;
            while (remainingPaddingToWrite--)
            {
                static const Byte kPadding[1] = {0};
                stream->write(kPadding, 1);
            }

            // Now we are at a suitably aligned offset.
            //
            totalSize = alignedSize;

            // With the alignment concern dealt with,
            // we can simply recursively write the
            // child chunk and update the total size.
            //
            SLANG_RETURN_ON_FAIL(child->_writeTo(stream));
            totalSize += child->_getCachedTotalSize();
        }

        // As a validation check, we expect the total number
        // of bytes we've written here to match the total
        // size that was cached on this chunk (and that was
        // written into the chunk header).
        //
        SLANG_ASSERT(totalSize == _getCachedTotalSize());
    }
    else
    {
        SLANG_UNREACHABLE("RIFF chunk must be data or list");
    }
    return SLANG_OK;
}

MemoryArena& ChunkBuilder::_getMemoryArena() const
{
    return getRIFFBuilder()->_getMemoryArena();
}

//
// RIFF::ListChunkBuilder
//

DataChunkBuilder* ListChunkBuilder::addDataChunk(Chunk::Type type)
{
    auto chunk = new (_getMemoryArena()) DataChunkBuilder(type, this);
    _children.add(chunk);
    return chunk;
}

ListChunkBuilder* ListChunkBuilder::addListChunk(Chunk::Type type)
{
    auto chunk = new (_getMemoryArena()) ListChunkBuilder(type, this);
    _children.add(chunk);
    return chunk;
}

//
// RIFF::DataChunkBuilder
//

void DataChunkBuilder::addData(void const* data, Size size)
{
    // Adding no data should be a no-op.
    //
    if (size == 0)
        return;

    // The most interesting implementation detail here
    // is that we will try to detect cases where we
    // can re-use an existing `Shard` by adding the data
    // to the end of that shard's allocation.
    //
    // This is only possible because of the way that
    // we are using a single `MemoryArena` to allocate
    // everything, which makes it possible that the
    // next address the arena would return for an allocation
    // of `size` bytes is the same as the ending address
    // of the payload for the last shard of this chunk.
    //
    auto& arena = _getMemoryArena();

    // We start by checking if this chunk already has
    // a last shard that we could consider appending to.
    //
    auto lastShard = _shards.getLast();
    if (lastShard)
    {
        // If there is a last shard, then we can compute
        // the end address of its payload, and see if
        // it is the same as the cursor of the arena
        // we are allocating from.
        //
        auto payload = lastShard->getPayload();
        auto payloadSize = lastShard->getPayloadSize();
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
                lastShard->setPayload(payload, payloadSize + size);
                return;
            }
        }
    }

    // If we didn't land in our special case, we
    // will simply allocate a new shard to hold
    // the data.
    //
    // Note that the order of allocation here is
    // intentional, and supports the optimized special
    // case that we checked for above. We make the
    // allocation for the payload *last*, so that
    // it is possible that the arena's next allocation
    // could come right after the payload allocation
    // in memory.
    //
    // If we allocated the payload first and the
    // `Shard` second, then there would already be
    // another allocation after the payload, and
    // the optimized case would never trigger.
    //
    auto shard = _addShard();
    auto payload = arena.allocateUnaligned(size);
    ::memcpy(payload, data, size);
    shard->setPayload(payload, size);
}

void DataChunkBuilder::addUnownedData(void const* data, size_t size)
{
    // Unowned data will always have to be added as its own shard.
    //
    auto shard = _addShard();
    shard->setPayload(data, size);
}

DataChunkBuilder::Shard* DataChunkBuilder::_addShard()
{
    auto shard = new (_getMemoryArena()) Shard();
    _shards.add(shard);
    return shard;
}

//
// RIFF::Builder
//

Builder::Builder()
    : _arena(4096)
{
}

Result Builder::writeTo(Stream* stream)
{
    // If there's no root chunk, then this isn't
    // a well-formed RIFF.
    //
    if (!_rootChunk)
        return SLANG_FAIL;

    // The `ChunkBuilder::_writeTo()` method requires size
    // information for each of the chunks in the hierarchy.
    // Rather than try to keep size information up-to-date
    // during the building process, we simply compute it
    // all at once, right before writing the output.
    //
    _rootChunk->_updateCachedTotalSize();
    _rootChunk->_writeTo(stream);
    return SLANG_OK;
}

Result Builder::writeToBlob(ISlangBlob** outBlob)
{
    OwnedMemoryStream stream(FileAccess::Write);
    SLANG_RETURN_ON_FAIL(writeTo(&stream));

    List<uint8_t> data;
    stream.swapContents(data);

    *outBlob = ListBlob::moveCreate(data).detach();
    return SLANG_OK;
}

ListChunkBuilder* Builder::addRootChunk(Chunk::Type type)
{
    // There must not already be a root chunk set.
    SLANG_ASSERT(getRootChunk() == nullptr);

    auto chunk = new (_getMemoryArena()) ListChunkBuilder(type, this);
    _rootChunk = chunk;
    return chunk;
}


//
// RIFF::BuildCursor
//

BuildCursor::BuildCursor() {}

BuildCursor::BuildCursor(Builder& builder)
    : _riffBuilder(&builder)
{
}

BuildCursor::BuildCursor(ChunkBuilder* chunk)
{
    setCurrentChunk(chunk);
}

void BuildCursor::setCurrentChunk(ChunkBuilder* chunk)
{
    _currentChunk = chunk;
    _riffBuilder = chunk ? chunk->getRIFFBuilder() : nullptr;
}

DataChunkBuilder* BuildCursor::addDataChunk(Chunk::Type type)
{
    // The current chunk must be a list chunk, so that
    // we can add children to it.
    //
    auto parentChunk = as<ListChunkBuilder>(getCurrentChunk());
    SLANG_ASSERT(parentChunk);

    return parentChunk->addDataChunk(type);
}

void BuildCursor::addDataChunk(Chunk::Type type, void const* data, size_t size)
{
    beginDataChunk(type);
    addData(data, size);
    endChunk();
}

ListChunkBuilder* BuildCursor::addListChunk(Chunk::Type type)
{
    // If there is no current chunk being written into,
    // then an attempt to add a new chunk should set
    // the root chunk of the entire RIFF.
    //
    auto currentChunk = getCurrentChunk();
    if (!currentChunk)
    {
        SLANG_ASSERT(getRIFFBuilder());
        return _riffBuilder->addRootChunk(type);
    }

    // Otherwise, the current chunk must be a list
    // chunk, and we add a new child to it.
    //
    auto parentChunk = as<ListChunkBuilder>(currentChunk);
    SLANG_ASSERT(parentChunk);

    return parentChunk->addListChunk(type);
}

void BuildCursor::beginDataChunk(Chunk::Type type)
{
    auto chunk = addDataChunk(type);
    setCurrentChunk(chunk);
}

void BuildCursor::beginListChunk(Chunk::Type type)
{
    auto chunk = addListChunk(type);
    setCurrentChunk(chunk);
}

void BuildCursor::endChunk()
{
    SLANG_ASSERT(getCurrentChunk() != nullptr);

    auto chunk = getCurrentChunk();
    setCurrentChunk(chunk->getParent());
}

void BuildCursor::addData(void const* data, Size size)
{
    // The current chunk must be a data chunk, so that
    // we can add data to it.
    //
    auto dataChunk = as<DataChunkBuilder>(getCurrentChunk());
    SLANG_ASSERT(dataChunk);

    dataChunk->addData(data, size);
}

void BuildCursor::addUnownedData(void const* data, Size size)
{
    // The current chunk must be a data chunk, so that
    // we can add data to it.
    //
    auto dataChunk = as<DataChunkBuilder>(getCurrentChunk());
    SLANG_ASSERT(dataChunk);

    dataChunk->addUnownedData(data, size);
}

} // namespace RIFF

} // namespace Slang
