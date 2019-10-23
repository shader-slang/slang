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
        // The rest of the header
        out->write(header + 1, headerSize - sizeof(RiffChunk));

        out->write(payload, payloadSize);
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

}
