// slang-fiddle-lua.cpp
#include "slang.h"

#define MAKE_LIB 1

#if SLANG_UNIX_FAMILY
#define LUA_USE_POSIX
#endif

#include "lua/onelua.c"
