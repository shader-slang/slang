// slang-fiddle-script.cpp
#include "slang-fiddle-script.h"

#include "lua/lapi.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

namespace fiddle
{
DiagnosticSink* _sink = nullptr;
StringBuilder* _builder = nullptr;
Count _templateCounter = 0;

static void _writeLuaMessage(Severity severity, String const& message)
{
    if (_sink)
    {
        _sink->diagnoseRaw(severity, message.getUnownedSlice());
    }
    else
    {
        fprintf(stderr, "%s", message.getBuffer());
    }
}

int _trace(lua_State* L)
{
    int argCount = lua_gettop(L);

    for (int i = 0; i < argCount; ++i)
    {
        lua_pushliteral(L, " ");
        luaL_tolstring(L, i + 1, nullptr);
    }
    lua_concat(L, 2 * argCount);

    size_t size = 0;
    char const* buffer = lua_tolstring(L, -1, &size);

    String message;
    message.append("fiddle:");
    message.append(UnownedStringSlice(buffer, size));
    message.append("\n");

    _writeLuaMessage(Severity::Note, message);
    return 0;
}

int _handleLuaErrorRaised(lua_State* L)
{
    lua_pushliteral(L, "\n");
    luaL_traceback(L, L, nullptr, 1);
    lua_concat(L, 3);
    return 1;
}

int _original(lua_State* L)
{
    // We ignore the text that we want to just pass
    // through unmodified...
    return 0;
}

int _raw(lua_State* L)
{
    size_t size = 0;
    char const* buffer = lua_tolstring(L, 1, &size);

    _builder->append(UnownedStringSlice(buffer, size));
    return 0;
}

int _splice(lua_State* L)
{
    auto savedBuilder = _builder;

    StringBuilder spliceBuilder;
    _builder = &spliceBuilder;

    lua_pushvalue(L, 1);
    lua_call(L, 0, 1);

    _builder = savedBuilder;

    // The actual string value follows whatever
    // got printed to the output (unless it is
    // nil).
    //
    _builder->append(spliceBuilder.produceString());
    if (!lua_isnil(L, -1))
    {
        size_t size = 0;
        char const* buffer = luaL_tolstring(L, -1, &size);
        _builder->append(UnownedStringSlice(buffer, size));
    }
    return 0;
}

int _template(lua_State* L)
{
    auto templateID = _templateCounter++;

    _builder->append("\n#if FIDDLE_GENERATED_OUTPUT_ID == ");
    _builder->append(templateID);
    _builder->append("\n");

    lua_pushvalue(L, 1);
    lua_call(L, 0, 0);

    _builder->append("\n#endif\n");

    return 0;
}

lua_State* L = nullptr;

void ensureLuaInitialized()
{
    if (L)
        return;

    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcclosure(L, &_trace, 0);
    lua_setglobal(L, "TRACE");

    lua_pushcclosure(L, &_original, 0);
    lua_setglobal(L, "ORIGINAL");

    lua_pushcclosure(L, &_raw, 0);
    lua_setglobal(L, "RAW");

    lua_pushcclosure(L, &_splice, 0);
    lua_setglobal(L, "SPLICE");

    lua_pushcclosure(L, &_template, 0);
    lua_setglobal(L, "TEMPLATE");

    // TODO: register custom stuff here...
}

lua_State* getLuaState()
{
    ensureLuaInitialized();
    return L;
}


String evaluateScriptCode(
    SourceLoc loc,
    String originalFileName,
    String scriptSource,
    DiagnosticSink* sink)
{
    StringBuilder builder;
    _builder = &builder;
    _templateCounter = 0;

    ensureLuaInitialized();

    String luaChunkName = "@" + originalFileName;

    lua_pushcfunction(L, &_handleLuaErrorRaised);

    if (LUA_OK != luaL_loadbuffer(
                      L,
                      scriptSource.getBuffer(),
                      scriptSource.getLength(),
                      luaChunkName.getBuffer()))
    {
        size_t size = 0;
        char const* buffer = lua_tolstring(L, -1, &size);
        String message = UnownedStringSlice(buffer, size);
        message = message + "\n";

        sink->diagnose(loc, fiddle::Diagnostics::scriptLoadError, message);
        SLANG_ABORT_COMPILATION("fiddle failed during Lua script loading");
    }

    if (LUA_OK != lua_pcall(L, 0, 0, -2))
    {
        size_t size = 0;
        char const* buffer = lua_tolstring(L, -1, &size);
        String message = UnownedStringSlice(buffer, size);
        message = message + "\n";

        sink->diagnose(loc, fiddle::Diagnostics::scriptExecutionError, message);
        SLANG_ABORT_COMPILATION("fiddle failed during Lua script execution");
    }

    _builder = nullptr;
    return builder.produceString();
}
} // namespace fiddle
