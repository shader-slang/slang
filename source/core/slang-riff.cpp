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
    stream->Seek(SeekOrigin::Current, chunkSize - sizeof(RiffChunk));
    return SLANG_OK;
}


/* static */SlangResult RiffUtil::readChunk(Stream* stream, RiffChunk& outChunk)
{
    try
    {
        stream->Read(&outChunk, sizeof(RiffChunk));
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    // TODO(JS): Could handle endianness issues here...

    return SLANG_OK;
}


/* static */SlangResult RiffUtil::writeData(uint32_t riffType, const void* data, size_t size, Stream* out)
{
    SLANG_ASSERT(uint64_t(size) <= uint64_t(0xfffffffff));

    // TODO(JS): Could handle endianness here
    RiffChunk chunk;
    chunk.m_type = riffType;
    chunk.m_size = uint32_t(size);

    try
    {
        out->Write(&chunk, sizeof(chunk));
        out->Write(data, size);
        size_t remaining = size & 3;
        if (remaining)
        {
            uint8_t end[4] = { 0, 0, 0, 0};
            out->Write(end, 4 - remaining);
        }
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}


/* static */SlangResult RiffUtil::readData(Stream* stream, RiffChunk& outChunk, List<uint8_t>& data)
{
    SLANG_RETURN_ON_FAIL(readChunk(stream, outChunk));

    data.setCount(outChunk.m_size);

    try
    {
        stream->Read(data.getBuffer(), outChunk.m_size);

        // Skip to the alignment
        uint32_t remaining = outChunk.m_size & 3;
        if (remaining)
        {
            stream->Seek(SeekOrigin::Current, 4 - remaining);
        }
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
}

}
