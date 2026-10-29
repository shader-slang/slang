#pragma once

#include <stdint.h>

namespace Slang
{
/// Identifies declarations with compiler-defined meaning independently of their source names.
enum class KnownBuiltinDeclName : uint32_t
{
#define SLANG_KNOWN_BUILTIN_DECL(NAME, VALUE) NAME = VALUE,
#include "slang-known-builtin-decl-name-defs.h"
#undef SLANG_KNOWN_BUILTIN_DECL
    COUNT
};
} // namespace Slang
