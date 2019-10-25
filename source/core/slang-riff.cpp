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
    m_dataChunk = nullptr;
}

void RiffContainer::reset()
{
    m_arena.reset();

    m_rootContainer = nullptr;
    m_container = nullptr;
    m_dataChunk = nullptr;
}

RiffContainer::ContainerChunk* RiffContainer::_newContainerChunk(FourCC type, FourCC subType)
{
    SLANG_ASSERT(RiffUtil::isContainerType(type));
    SLANG_ASSERT(!RiffUtil::isContainerType(subType));

    ContainerChunk* chunk = (ContainerChunk*)m_arena.allocate(sizeof(ContainerChunk));
    chunk->init(type, subType);
    return chunk;
}

RiffContainer::DataChunk* RiffContainer::_newDataChunk(FourCC type)
{
    SLANG_ASSERT(!RiffUtil::isContainerType(type));

    DataChunk* chunk = (DataChunk*)m_arena.allocate(sizeof(DataChunk));
    chunk->init(type);
    return chunk;
}

void RiffContainer::_addChunk(Chunk* chunk)
{
    if (m_container)
    {
        chunk->m_parent = m_container;
        Chunk*& next = m_container->m_endChunk ? m_container->m_endChunk->m_next : m_container->m_containedChunks;
        SLANG_ASSERT(next == nullptr);
        next = chunk;
        m_container->m_endChunk = chunk;
    }
}

void RiffContainer::startChunk(Chunk::Kind kind, FourCC fourCC)
{
    SLANG_ASSERT(m_container || m_rootContainer == nullptr);

    switch (kind)
    {
        case Chunk::Kind::Data:
        {
            // We can only start a data chunk if we are in a container, and we can't already be in data chunk
            SLANG_ASSERT(m_container && m_dataChunk == nullptr);

            DataChunk* chunk = _newDataChunk(fourCC);
            _addChunk(chunk);
            m_dataChunk = chunk;
            break;
        }
        case Chunk::Kind::Container:
        {
            // We can't be in a data chunk
            SLANG_ASSERT(m_dataChunk == nullptr);

            ContainerChunk* container = _newContainerChunk(RiffFourCC::kList, fourCC);

            // If this is the first, make it the root
            if (!m_rootContainer)
            {
                m_rootContainer = container;
            }

            _addChunk(container);

            m_container = container;
            break;
        }
    }
}

void RiffContainer::endChunk()
{
    size_t chunkPayloadSize;

    ContainerChunk* parent;
    if (m_dataChunk)
    {
        parent = m_dataChunk->m_parent;
        chunkPayloadSize = m_dataChunk->m_payloadSize;

        m_dataChunk = nullptr;
    }
    else
    {
        SLANG_ASSERT(m_container && m_dataChunk == nullptr);
        parent = m_container->m_parent;
        chunkPayloadSize = m_container->m_payloadSize;
    }

    m_container = parent;


    
    if (parent)
    {
        // Fix the size taking into account the 4 bytes alignment requirement
        chunkPayloadSize = (chunkPayloadSize + 3) & ~size_t(3);

        // Update the parents size
        parent->m_payloadSize += sizeof(RiffChunk) + chunkPayloadSize;
    }
}

RiffContainer::Data* RiffContainer::addData(size_t size)
{
    // We must be in a chunk
    SLANG_ASSERT(m_dataChunk);

    // Add current chunks data
    m_dataChunk->m_payloadSize += size;

    Data* data = (Data*)m_arena.allocate(sizeof(Data) + size);

    data->m_next = nullptr;
    data->m_size = size;

    Data*& next = m_dataChunk->m_endData ? m_dataChunk->m_endData->m_next : m_dataChunk->m_dataList;
    SLANG_ASSERT(next == nullptr);

    // Add to linked list
    next = data;
    // Make this the new end
    m_dataChunk->m_endData = data;
    return data;
}

void RiffContainer::write(const void* inData, size_t size)
{
    auto data = addData(size);
    ::memcpy(data->getPayload(), inData, size);
}

/* static */bool RiffContainer::isContainerOk(ContainerChunk* container)
{
    // Check the size

    size_t totalSize = sizeof(RiffContainerHeader) - sizeof(RiffChunk);
    Chunk* chunk = container->m_containedChunks;
    while (chunk)
    {
        if (ContainerChunk* contChunk = as<ContainerChunk>(chunk))
        {
            if (!isContainerOk(contChunk))
            {
                return false;
            }
        }
        else if (DataChunk* dataChunk = as<DataChunk>(chunk))
        {
            size_t chunkSize = 0;
            // Work out what contained size is
            Data* data = dataChunk->m_dataList;
            while (data)
            {
                chunkSize += data->m_size;
                data = data->m_next;
            }

            if (chunkSize != chunk->m_payloadSize)
            {
                return false;
            }
        }

        const size_t payloadSize = (chunk->m_payloadSize + 3) & ~size_t(3);

        totalSize += sizeof(RiffChunk) + payloadSize;
        chunk = chunk->m_next;
    }

    return totalSize == container->m_payloadSize;
}

/* static */SlangResult RiffContainer::write(ContainerChunk* container, bool isRoot, Stream* stream)
{
    RiffContainerHeader containerHeader;

    containerHeader.chunk.m_type = isRoot ? RiffFourCC::kRiff : RiffFourCC::kList;
    containerHeader.chunk.m_size = uint32_t(sizeof(RiffContainerHeader) - sizeof(RiffChunk) + container->m_payloadSize);
    containerHeader.subType = container->m_subType;

    try
    {
        // Write the header
        stream->write(&containerHeader, sizeof(containerHeader));

        // Write the contained chunks
        Chunk* chunk = container->m_containedChunks;
        while (chunk)
        {
            if (auto containerChunk = as<ContainerChunk>(chunk))
            {
                // It's a container
                SLANG_RETURN_ON_FAIL(write(containerChunk, false, stream));
            }
            else if (auto dataChunk = as<DataChunk>(chunk))
            {
                // Must be a regular chunk with data

                RiffChunk chunkHeader;
                chunkHeader.m_type = dataChunk->m_type;
                chunkHeader.m_size = uint32_t(dataChunk->m_payloadSize);

                stream->write(&chunkHeader, sizeof(chunkHeader));

                Data* data = dataChunk->m_dataList;
                while (data)
                {
                    stream->write(data->getPayload(), data->getSize());

                    // Next but of data
                    data = data->m_next;
                }
            }

            // Next
            chunk = chunk->m_next;
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

        outContainer.startChunk(Chunk::Kind::Container, header.subType);
    }

    List<size_t> remainingStack;
    while (true)
    {
        if (remaining == 0)
        {
            // If it's a container then we pop container
            outContainer.endChunk();
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
                outContainer.startChunk(Chunk::Kind::Container, header.subType);
            }
            else
            {
                ScopeChunk scopeChunk(&outContainer, Chunk::Kind::Data, header.chunk.m_type);
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
