// slang-fiddle-script.h
#pragma once

#include "compiler-core/slang-source-loc.h"
#include "core/slang-list.h"
#include "core/slang-string.h"
#if SLANG_USE_SYSTEM_LUA
#include <lua.hpp>
#else
#include "lua/lapi.h"
#include "lua/lauxlib.h"
#endif
#include "slang-fiddle-diagnostics.h"
#include "slang-fiddle-scrape.h"

namespace fiddle
{
using namespace Slang;

lua_State* getLuaState();

String evaluateScriptCode(
    SourceLoc loc,
    String originalFileName,
    String scriptSource,
    DiagnosticSink* sink);

String evaluateLuaExpression(
    SourceLoc loc,
    String originalFileName,
    String scriptSource,
    DiagnosticSink* sink);
} // namespace fiddle
