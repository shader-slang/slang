// slang-ir-version-check-lua.cpp
//
// Compiles the vendored Lua interpreter into this tool, mirroring
// tools/slang-fiddle/slang-fiddle-lua.cpp. The stable-names table
// (source/slang/slang-ir-insts-stable-names.lua) is a Lua chunk, so the most
// robust way to read it — handling quoting/escaping/formatting that ad-hoc text
// parsing would get wrong — is to execute it in an embedded interpreter.
#include "slang.h"

#define MAKE_LIB 1

#if SLANG_UNIX_FAMILY
#define LUA_USE_POSIX
#endif

#include "lua/onelua.c"
