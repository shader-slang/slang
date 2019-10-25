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
        stream->read(outHeader + 1, headerSize - sizeof(RiffChunk));

        const size_t payloadSize = chunk.m_size - (headerSize - sizeof(RiffChunk));

        data.setCount(payloadSize);

        stream->read(data.getBuffer(), payloadSize);

        // Skip to the alignment
        uint32_t remaining = payloadSize & 3;
        if (remaining)
        {
            stream->seek(SeekOrigin::Current, 4 - remaining);
        }
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
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

/* static */SlangResult RiffUtil::readContainer(Stream* stream, RiffContainerHeader& outHeader, MemoryArena& ioArena, List<SubChunk>& outChunks)
{
    outChunks.clear();

    // Need to read the chunk header
    SLANG_RETURN_ON_FAIL(readChunk(stream, outHeader.chunk));

    // Must be a riff container!
    if (outHeader.chunk.m_type != RiffFourCC::kRiff && outHeader.chunk.m_type != RiffFourCC::kList)
    {
        return SLANG_FAIL;
    }

    // Read the sub type
    try
    {
        stream->read(&outHeader.subType, sizeof(RiffContainerHeader) - sizeof(RiffChunk));
    }
    catch (const IOException&)
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

}
