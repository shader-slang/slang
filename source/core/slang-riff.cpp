#include "slang-riff.h"

#include "../../slang-com-helper.h"

namespace Slang
{

/* static */int64_t RiffUtil::calcChunkTotalSize(const RiffChunk& chunk)
{
    int64_t size = chunk.m_size + sizeof(RiffChunk);
    return (size + 3) & ~int64_t(3);
}

/* static */SlangResult RiffUtil::skip(const RiffChunk& chunk, Stream* stream, int64_t* remainingBytesInOut)
{
    int64_t chunkSize = calcChunkTotalSize(chunk);
    if (remainingBytesInOut)
    {
        *remainingBytesInOut -= chunkSize;
    }

    // Skip the payload (we don't need to skip the Chunk because that was already read
    stream->seek(SeekOrigin::Current, chunkSize - sizeof(RiffChunk));
    return SLANG_OK;
}


/* static */SlangResult RiffUtil::readChunk(Stream* stream, RiffChunk& outChunk)
{
    try
    {
        stream->read(&outChunk, sizeof(RiffChunk));
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    // TODO(JS): Could handle endianness issues here...

    return SLANG_OK;
}


/* static */SlangResult RiffUtil::writeData(const RiffChunk* header, size_t headerSize, const void* payload, size_t payloadSize, Stream* out)
{
    SLANG_ASSERT(uint64_t(payloadSize) <= uint64_t(0xfffffffff));
    SLANG_ASSERT(headerSize >= sizeof(RiffChunk));
    SLANG_ASSERT((headerSize & 3) == 0);

    // TODO(JS): Could handle endianness here

    RiffChunk chunk;
    chunk.m_type = header->m_type;
    chunk.m_size = uint32_t(headerSize - sizeof(RiffChunk) + payloadSize);

    try
    {
        // The chunk
        out->write(&chunk, sizeof(RiffChunk));

        // Remainder of header
        if (headerSize > sizeof(RiffChunk))
        {
            // The rest of the header
            out->write(header + 1, headerSize - sizeof(RiffChunk));
        }

        // Write the payload
        out->write(payload, payloadSize);

        // The riff spec requires all chunks are 4 byte aligned (even if size is not)
        size_t remaining = payloadSize & 3;
        if (remaining)
        {
            uint8_t end[4] = { 0, 0, 0, 0};
            out->write(end, 4 - remaining);
        }
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readPayload(Stream* stream, size_t size, void* outData, size_t& outReadSize)
{
    outReadSize = 0;
    try
    {
        stream->read(outData, size);
        const size_t readSize = (size + 3) & ~size_t(3);

        // Skip to the alignment
        if (readSize > size)
        {
            stream->seek(SeekOrigin::Current, readSize - size);
        }
        outReadSize = readSize;
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readData(Stream* stream, RiffChunk* outHeader, size_t headerSize, List<uint8_t>& data)
{
    RiffChunk chunk;
    SLANG_RETURN_ON_FAIL(readChunk(stream, chunk));
    if (chunk.m_size < headerSize)
    {
        return SLANG_FAIL;
    }

    *outHeader = chunk;

    try
    {
        // Read the header
        if (headerSize > sizeof(RiffChunk))
        {
            stream->read(outHeader + 1, headerSize - sizeof(RiffChunk));
        }
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    const size_t payloadSize = chunk.m_size - (headerSize - sizeof(RiffChunk));
    size_t readSize;
    data.setCount(payloadSize);
    return readPayload(stream, payloadSize, data.getBuffer(), readSize);
}

/* static */SlangResult RiffUtil::writeContainerHeader(FourCC containerType, FourCC subType, size_t totalSize, Stream* out)
{
    RiffContainerHeader header;

    // Container can only be list or riff - they are the only available options
    SLANG_ASSERT(containerType == RiffFourCC::kList || containerType == RiffFourCC::kRiff);
    if (!(containerType == RiffFourCC::kList || containerType == RiffFourCC::kRiff))
    {
        return SLANG_FAIL;
    }

    // Size of header of Riff container
    totalSize += sizeof(RiffContainerHeader) - sizeof(RiffChunk);

    header.chunk.m_type = containerType;
    header.chunk.m_size = uint32_t(totalSize);
    header.subType = subType;
    
    try
    {
        out->write(&header, sizeof(header));
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::writeContainer(FourCC containerType, FourCC subType, const SubChunk* subChunks, size_t subChunkCount, Stream* out)
{
    size_t totalSize = 0;

    for (size_t i = 0; i < subChunkCount; ++i)
    {
        const auto& subChunk = subChunks[i];

        RiffChunk chunk;
        chunk.m_type = subChunk.chunkType;
        SLANG_ASSERT(subChunk.dataSize <= 0xffffffff);
        chunk.m_size = uint32_t(subChunk.dataSize);

        totalSize += calcChunkTotalSize(chunk);
    }

    // Write the container header
    SLANG_RETURN_ON_FAIL(writeContainerHeader(containerType, subType, totalSize, out));

    for (size_t i = 0; i < subChunkCount; ++i)
    {
        const auto& subChunk = subChunks[i];

        RiffChunk chunk;
        chunk.m_type = subChunk.chunkType;
        SLANG_ASSERT(subChunk.dataSize <= 0xffffffff);
        chunk.m_size = uint32_t(subChunk.dataSize);

        // Write the chunk
        SLANG_RETURN_ON_FAIL(writeData(&chunk, sizeof(chunk), subChunk.data, subChunk.dataSize, out));
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readHeader(Stream* stream, RiffContainerHeader& outHeader)
{
    // Need to read the chunk header
    SLANG_RETURN_ON_FAIL(readChunk(stream, outHeader.chunk));
    outHeader.subType = 0;

    if (isContainerType(outHeader.chunk.m_type))
    {
        // Read the sub type
        try
        {
            stream->read(&outHeader.subType, sizeof(RiffContainerHeader) - sizeof(RiffChunk));
        }
        catch (const IOException&)
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readContainer(Stream* stream, RiffContainerHeader& outHeader, MemoryArena& ioArena, List<SubChunk>& outChunks)
{
    outChunks.clear();
    // Need to read the chunk header
    SLANG_RETURN_ON_FAIL(readHeader(stream, outHeader));

    // Must be a riff container!
    if (!isContainerType(outHeader.chunk.m_type))
    {
        return SLANG_FAIL;
    }

    // Okay we have the header. We now need to read all of the contained chunks. Making sure we don't read past the end
    size_t remainingSize = size_t(outHeader.chunk.m_size) - (sizeof(RiffContainerHeader) - sizeof(RiffChunk));
    while (remainingSize >= sizeof(RiffChunk))
    {
        // Read the contained chunk
        RiffChunk chunk;
        SLANG_RETURN_ON_FAIL(readChunk(stream, chunk));

        remainingSize -= sizeof(RiffChunk);
        if (remainingSize < 0 || chunk.m_size > remainingSize)
        {
            return SLANG_FAIL;
        }
        
        // Allocate the space for the payload
        void* data = ioArena.allocate(chunk.m_size);

        // Read the payload
        try
        {
            stream->read(data, chunk.m_size);
        }
        catch (const IOException&)
        {
            return SLANG_FAIL;
        }

        // Add to the list
        SubChunk subChunk;
        subChunk.chunkType = chunk.m_type;
        subChunk.data = data;
        subChunk.dataSize = chunk.m_size;

        // Decrease the remaining size
        remainingSize -= chunk.m_size;
    }

    // Remaining size should be 0 for well formed riff
    SLANG_ASSERT(remainingSize == 0);

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RiffContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

RiffContainer::RiffContainer() :
    m_arena(4096)
{
    m_rootContainer = nullptr;
    m_container = nullptr;
    m_endChunk = nullptr;
    m_endData = nullptr;
}

void RiffContainer::reset()
{
    m_arena.reset();

    m_rootContainer = nullptr;
    m_container = nullptr;
    m_endChunk = nullptr;
    m_endData = nullptr;
}

void RiffContainer::_initAndAddChunk(FourCC chunkType, Chunk* chunk)
{    
    chunk->type = chunkType;
    chunk->next = nullptr;
    chunk->dataList = nullptr;
    chunk->size = 0;

    if (m_endChunk)
    {
        m_endChunk->next = chunk;
        m_endChunk = chunk;
    }
    else if (m_container)
    {
        // It can't have any set
        SLANG_ASSERT(m_container->containedChunks == nullptr);

        // Make this the fist in this container
        m_container->containedChunks = chunk;
        m_endChunk = chunk;
    }
}

void RiffContainer::startChunk(FourCC chunkType)
{
    // Can't be a container
    SLANG_ASSERT(!RiffUtil::isContainerType(chunkType));

    // We can only start a chunk if we are in a container
    SLANG_ASSERT(m_container);
    // Can't be in a regular chunk
    SLANG_ASSERT(m_endData == nullptr);

    _initAndAddChunk(chunkType, (Chunk*)m_arena.allocate(sizeof(Chunk)));
}

RiffContainer::Data* RiffContainer::addData(size_t size)
{
    // We must be in a chunk
    SLANG_ASSERT(m_endChunk);

    // Add current chunks data
    m_endChunk->size += size;

    Data* data = (Data*)m_arena.allocate(sizeof(Data) + size);

    data->next = nullptr;
    data->size = size;

    // If there is a previous, just add to the end
    if (m_endData)
    {
        m_endData->next = data;
    }
    else
    {
        // Set as the first in the chunk
        SLANG_ASSERT(m_endChunk->dataList == nullptr);
        m_endChunk->dataList = data;
    }

    // This is the new end
    m_endData = data;

    return data;
}

void RiffContainer::write(const void* inData, size_t size)
{
    auto data = addData(size);
    ::memcpy(data->getPayload(), inData, size);
}

void RiffContainer::endChunk()
{
    // There must be an end chunk
    SLANG_ASSERT(m_endChunk);

    // We can add the size to the container
    const size_t chunkContentsSize = (m_endChunk->size + 3) & ~size_t(3);
    m_container->size += sizeof(RiffChunk) + chunkContentsSize;

    // We can reset data
    m_endData = nullptr;
}

void RiffContainer::startContainer(FourCC subType)
{
    // Can't be *in* a chunk
    SLANG_ASSERT(m_endData == nullptr);
    // Either there is a root container and there is a container
    // or there is no root container, and therefore also no container
    SLANG_ASSERT((m_rootContainer && m_container) || (m_rootContainer == nullptr && m_container == nullptr));

    Container* container = (Container*)m_arena.allocate(sizeof(Container));

    container->containedChunks = nullptr;
    container->subType = subType;  
    container->endChunk = nullptr;

    // We'll store all containers as list. Only when writing - we will decide to make the root one a 'riff'
    _initAndAddChunk(RiffFourCC::kList, container);

    // If there is a container, save it
    if (m_container)
    {
        // Save off in the container, the last chunk
        m_container->endChunk = m_endChunk;
        m_containerStack.add(m_container);
    }
    else
    {
        // Can't be a root
        SLANG_ASSERT(m_rootContainer == nullptr);
        // Save as the root
        m_rootContainer = container;
    }

    // This now becomes the current container
    m_container = container;
    m_endChunk = nullptr;
}

void RiffContainer::endContainer()
{
    if (m_containerStack.getCount())
    {
        Container* container = m_containerStack.getLast();
        m_containerStack.removeLast();

        m_container = container;
        m_endChunk = container->endChunk;
    }
    else
    {
        m_container = nullptr;
        m_endChunk = nullptr;
    }
}


/* static */bool RiffContainer::isContainerOk(Container* container)
{
    // Check the size

    size_t totalSize = sizeof(RiffContainerHeader) - sizeof(RiffChunk);
    Chunk* chunk = container->containedChunks;
    while (chunk)
    {
        if (RiffUtil::isContainerType(chunk->type))
        {
            if (!isContainerOk(static_cast<Container*>(chunk)))
            {
                return false;
            }
        }
        else
        {
            size_t chunkSize = 0;
            // Work out what contained size is
            Data* data = chunk->dataList;
            while (data)
            {
                chunkSize += data->size;
                data = data->next;
            }

            if (chunkSize != chunk->size)
            {
                return false;
            }
        }

        totalSize += sizeof(RiffChunk) + chunk->size;
        chunk = chunk->next;
    }

    return totalSize == container->size;
}

/* static */SlangResult RiffContainer::write(Container* container, bool isRoot, Stream* stream)
{
    RiffContainerHeader containerHeader;

    containerHeader.chunk.m_type = isRoot ? RiffFourCC::kRiff : RiffFourCC::kList;
    containerHeader.chunk.m_size = uint32_t(sizeof(RiffContainerHeader) - sizeof(RiffChunk) + container->size);
    containerHeader.subType = container->subType;

    try
    {
        // Write the header
        stream->write(&containerHeader, sizeof(containerHeader));

        // Write the contained chunks
        Chunk* chunk = container->containedChunks;
        while (chunk)
        {
            if (chunk->type == RiffFourCC::kList)
            {
                // It's a container
                SLANG_RETURN_ON_FAIL(write(static_cast<Container*>(chunk), false, stream));
            }
            else
            {
                // Must be a regular chunk with data

                RiffChunk chunkHeader;
                chunkHeader.m_type = chunk->type;
                chunkHeader.m_size = uint32_t(chunk->size);

                stream->write(&chunkHeader, sizeof(chunkHeader));

                Data* data = chunk->dataList;
                while (data)
                {
                    stream->write(data->getPayload(), data->getSize());

                    // Next but of data
                    data = data->next;
                }
            }

            // Next
            chunk = chunk->next;
        }
    }
    catch (const IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffContainer::read(Stream* stream, RiffContainer& outContainer)
{
    typedef RiffContainer::ScopeChunk ScopeChunk;
    typedef RiffContainer::ScopeChunk ScopeContainer;
    outContainer.reset();

    size_t remaining;
    {
        RiffContainerHeader header;
        SLANG_RETURN_ON_FAIL(RiffUtil::readHeader(stream, header));
        if (!RiffUtil::isContainerType(header.chunk.m_type))
        {
            return SLANG_FAIL;
        }

        remaining = header.chunk.m_size - (sizeof(RiffContainerHeader) - sizeof(RiffChunk));

        outContainer.startContainer(header.subType);
    }

    List<size_t> remainingStack;
    while (true)
    {
        if (remaining == 0)
        {
            // If it's a container then we pop container
            outContainer.endContainer();
            if (remainingStack.getCount() <= 0)
            {
                break;
            }

            remaining = remainingStack.getLast();
            remainingStack.removeLast();
        }
        else
        {
            RiffContainerHeader header;
            SLANG_RETURN_ON_FAIL(RiffUtil::readHeader(stream, header));

            if (header.chunk.m_type == RiffFourCC::kList)
            {
                // Subtract the size of this chunk from remaining
                remaining -= sizeof(RiffContainerHeader);
                remaining -= (header.chunk.m_size + 3) & ~size_t(3);

                // Push it, for when we hit the end
                remainingStack.add(remaining);

                // Work out how much remains in this container
                remaining -= (header.chunk.m_size - (sizeof(RiffContainerHeader) - sizeof(RiffChunk)) +  3) & ~size_t(3);

                // Start a container
                outContainer.startContainer(header.subType);
            }
            else
            {
                ScopeChunk scopeChunk(&outContainer, header.chunk.m_type);
                Data* data = outContainer.addData(header.chunk.m_size);

                size_t readSize;
                SLANG_RETURN_ON_FAIL(RiffUtil::readPayload(stream, header.chunk.m_size, data->getPayload(), readSize));

                // Correct remaining
                remaining -= sizeof(RiffChunk) + readSize;
            }
        }
    }

    return outContainer.isFullyConstructed() ? SLANG_OK : SLANG_FAIL;
}

}
