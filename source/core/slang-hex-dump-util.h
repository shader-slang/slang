#ifndef SLANG_HEX_DUMP_UTIL_H
#define SLANG_HEX_DUMP_UTIL_H

#include "slang-common.h"
#include "slang-string.h"

#include "slang-list.h"
#include "../../slang.h"

namespace Slang
{

struct HexDumpUtil
{
        /// Dump data to writer, with lines starting with hex data
    static SlangResult dump(const List<uint8_t>& data, int numBytesPerLine, ISlangWriter* writer);

    static SlangResult dumpWithMarkers(const List<uint8_t>& data, int numBytesPerLine, ISlangWriter* writer);

        /// Parses lines formatted by dump, back into bytes
    static SlangResult parse(const UnownedStringSlice& lines, List<uint8_t>& outBytes);

    static SlangResult parseWithMarkers(const UnownedStringSlice& lines, List<uint8_t>& outBytes);
};

}

#endif
