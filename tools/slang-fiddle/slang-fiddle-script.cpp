// slang-fiddle-script.cpp
#include "slang-fiddle-script.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "lua/lapi.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include <cstdio>

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

// Add a custom searcher that handles relative paths
// So we can do things like require("source/slang/foo.lua")
static int path_searcher(lua_State* L)
{
    const char* modname = luaL_checkstring(L, 1);

    if (luaL_loadfile(L, modname) == LUA_OK)
    {
        lua_pushstring(L, modname); // Push filename as second return
        return 2;
    }

    // Not found
    lua_pushfstring(L, "\n\tno file '%s'", modname);
    return 1;
}

void install_path_searcher(lua_State* L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers");

    // Insert at position 2 (after preload)
    lua_pushcfunction(L, path_searcher);
    lua_rawseti(L, -2, 2);

    lua_pop(L, 2);
}

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

    install_path_searcher(L);

    // TODO: register custom stuff here...
}

lua_State* getLuaState()
{
    ensureLuaInitialized();
    return L;
}

static void setupLuaEnvironment(const String& originalFileName)
{
    ensureLuaInitialized();

    lua_pushstring(L, originalFileName.getBuffer());
    lua_setglobal(L, "THIS_FILE");

    lua_pushcfunction(L, &_handleLuaErrorRaised);
}

static void handleLuaError(
    SourceLoc loc,
    DiagnosticSink* sink,
    const char* errorType,
    DiagnosticInfo diagnosticID)
{
    size_t size = 0;
    char const* buffer = lua_tolstring(L, -1, &size);
    String message = UnownedStringSlice(buffer, size);
    message = message + "\n";

    sink->diagnose(loc, diagnosticID, message);

    String abortMessage = "fiddle failed during Lua ";
    abortMessage = abortMessage + errorType;
    SLANG_ABORT_COMPILATION(abortMessage.getBuffer());
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

    setupLuaEnvironment(originalFileName);

    String luaChunkName = "@" + originalFileName;

    if (LUA_OK != luaL_loadbuffer(
                      L,
                      scriptSource.getBuffer(),
                      scriptSource.getLength(),
                      luaChunkName.getBuffer()))
    {
        handleLuaError(loc, sink, "script loading", fiddle::Diagnostics::scriptLoadError);
    }

    if (LUA_OK != lua_pcall(L, 0, 0, -2))
    {
        handleLuaError(loc, sink, "script execution", fiddle::Diagnostics::scriptExecutionError);
    }

    _builder = nullptr;
    return builder.produceString();
}

String evaluateLuaExpression(
    SourceLoc loc,
    String originalFileName,
    String luaExpression,
    DiagnosticSink* sink)
{
    setupLuaEnvironment(originalFileName);

    String luaChunkName = "@" + originalFileName;

    // Wrap expression in return statement to get its value
    String wrappedExpression = "return " + luaExpression;

    if (LUA_OK != luaL_loadbuffer(
                      L,
                      wrappedExpression.getBuffer(),
                      wrappedExpression.getLength(),
                      luaChunkName.getBuffer()))
    {
        handleLuaError(loc, sink, "expression loading", fiddle::Diagnostics::scriptLoadError);
    }

    // Execute and expect 1 return value
    if (LUA_OK != lua_pcall(L, 0, 1, -2))
    {
        handleLuaError(
            loc,
            sink,
            "expression evaluation",
            fiddle::Diagnostics::scriptExecutionError);
    }

    // Convert the result to string
    size_t resultSize = 0;
    const char* resultBuffer = lua_tolstring(L, -1, &resultSize);

    if (!resultBuffer)
    {
        sink->diagnose(
            loc,
            fiddle::Diagnostics::scriptExecutionError,
            "Lua expression did not return a string value\n");
        SLANG_ABORT_COMPILATION("fiddle failed: non-string expression result");
    }

    String result;
    result.append(resultBuffer, resultSize);

    // Pop the result and error handler
    lua_pop(L, 2);

    return result;
}
} // namespace fiddle
