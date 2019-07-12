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
    static SlangResult dump(const List<uint8_t>& data, int numBytesPerLine, ISlangWriter* writer);

};

}

#endif
