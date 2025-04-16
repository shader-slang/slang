// slang-fiddle-script.h
#pragma once

#include "slang-fiddle-diagnostics.h"
#include "slang-fiddle-scrape.h"

#include "core/slang-string.h"
#include "core/slang-list.h"
#include "compiler-core/slang-source-loc.h"

#include "../external/lua/lapi.h"
#include "../external/lua/lauxlib.h"

namespace fiddle
{
    using namespace Slang;

    lua_State* getLuaState();

    String evaluateScriptCode(
        String originalFileName,
        String scriptSource,
        DiagnosticSink* sink);
}
