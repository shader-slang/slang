#ifndef SLANG_STRING_UTIL_H
#define SLANG_STRING_UTIL_H

#include "slang-string.h"
#include "list.h"

namespace Slang {

struct StringUtil
{
    static void split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& slicesOut);
};

} // namespace Slang


#endif // SLANG_STRING_UTIL_H
