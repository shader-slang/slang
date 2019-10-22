#ifndef SLANG_RIFF_H
#define SLANG_RIFF_H

#include "../core/slang-basic.h"
#include "../core/slang-stream.h"

namespace Slang
{

// http://fileformats.archiveteam.org/wiki/RIFF
// http://www.fileformat.info/format/riff/egff.htm

#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 0) | (uint32_t(c1) << 8) | (uint32_t(c2) << 16) | (uint32_t(c3) << 24)) 

struct RiffFourCC
{
     static const uint32_t kRiff = SLANG_FOUR_CC('R', 'I', 'F', 'F');
};

struct RiffChunk
{
    uint32_t m_type;            ///< The FourCC code that identifies this chunk
    uint32_t m_size;            ///< Size does *NOT* include the riff chunk size. The size can be byte sized, but on storage it will always be treated as aligned up by 4.
};

struct RiffUtil
{
    static int64_t calcChunkTotalSize(const RiffChunk& chunk);

    static SlangResult skip(const RiffChunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    static SlangResult readChunk(Stream* stream, RiffChunk& outChunk);


    static SlangResult writeData(uint32_t riffType, const void* data, size_t size, Stream* out);
    static SlangResult readData(Stream* stream, RiffChunk& outChunk, List<uint8_t>& data);
};

}

#endif
