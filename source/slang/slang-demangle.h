// slang-demangle.h
#ifndef SLANG_DEMANGLE_H_INCLUDED
#define SLANG_DEMANGLE_H_INCLUDED

#include "../core/slang-basic.h"

namespace Slang
{
struct DemangleResult
{
    bool success = false;
    String text;
};

SLANG_API bool tryDemangleName(const UnownedStringSlice& mangledName, String& outDemangled);
SLANG_API String demangleName(const UnownedStringSlice& mangledName);

} // namespace Slang

#endif
