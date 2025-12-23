// slang-fiddle-diagnostic-gen.cpp
#include "slang-fiddle-diagnostic-gen.h"

#include "core/slang-io.h"
#include "lua/lapi.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "slang-fiddle-script.h"

namespace fiddle
{

bool DiagnosticGenerator::loadFromLuaFile(const String& filePath)
{
    // Read the Lua file
    String luaSource;
    if (SLANG_FAILED(File::readAllText(filePath, luaSource)))
    {
        return false;
    }

    // Get the Lua state from the fiddle script system
    lua_State* L = getLuaState();

    // Create a table to collect diagnostic definitions
    lua_newtable(L);
    int diagnosticsTable = lua_gettop(L);

    // Define the 'diagnostic' function that will be called from Lua
    // This captures definitions into our table
    lua_pushlightuserdata(L, &m_definitions);
    lua_pushcclosure(
        L,
        [](lua_State* L) -> int
        {
            // Get the definitions list from upvalue
            auto* definitions =
                static_cast<List<DiagnosticDef>*>(lua_touserdata(L, lua_upvalueindex(1)));

            // First argument is the diagnostic name
            const char* name = luaL_checkstring(L, 1);

            // Second argument is the definition table
            luaL_checktype(L, 2, LUA_TTABLE);

            DiagnosticDef def;
            def.name = name;

            // Extract fields from the table
            lua_getfield(L, 2, "code");
            if (lua_isstring(L, -1))
                def.code = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "severity");
            if (lua_isstring(L, -1))
                def.severity = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "flag");
            if (lua_isstring(L, -1))
                def.flag = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "message");
            if (lua_isstring(L, -1))
                def.message = lua_tostring(L, -1);
            lua_pop(L, 1);

            // Extract params array
            lua_getfield(L, 2, "params");
            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    if (lua_istable(L, -1))
                    {
                        DiagnosticParam param;
                        lua_getfield(L, -1, "name");
                        if (lua_isstring(L, -1))
                            param.name = lua_tostring(L, -1);
                        lua_pop(L, 1);

                        lua_getfield(L, -1, "type");
                        if (lua_isstring(L, -1))
                            param.type = lua_tostring(L, -1);
                        lua_pop(L, 1);

                        def.params.add(param);
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            // Extract primary_label
            lua_getfield(L, 2, "primary_label");
            if (lua_istable(L, -1))
            {
                lua_getfield(L, -1, "loc");
                if (lua_isstring(L, -1))
                    def.primaryLabel.locName = lua_tostring(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "message");
                if (lua_isstring(L, -1))
                    def.primaryLabel.message = lua_tostring(L, -1);
                lua_pop(L, 1);

                def.primaryLabel.isPrimary = true;
            }
            lua_pop(L, 1);

            // Extract secondary_labels array
            lua_getfield(L, 2, "secondary_labels");
            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    if (lua_istable(L, -1))
                    {
                        DiagnosticLabelDef label;
                        lua_getfield(L, -1, "loc");
                        if (lua_isstring(L, -1))
                            label.locName = lua_tostring(L, -1);
                        lua_pop(L, 1);

                        lua_getfield(L, -1, "message");
                        if (lua_isstring(L, -1))
                            label.message = lua_tostring(L, -1);
                        lua_pop(L, 1);

                        label.isPrimary = false;
                        def.secondaryLabels.add(label);
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            // Extract notes array
            lua_getfield(L, 2, "notes");
            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    if (lua_isstring(L, -1))
                        def.notes.add(lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            // Extract helps array
            lua_getfield(L, 2, "helps");
            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    if (lua_isstring(L, -1))
                        def.helps.add(lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            definitions->add(def);
            return 0;
        },
        1);
    lua_setglobal(L, "diagnostic");

    // Execute the Lua file
    String chunkName = "@" + filePath;
    if (luaL_loadbuffer(L, luaSource.getBuffer(), luaSource.getLength(), chunkName.getBuffer()) !=
        LUA_OK)
    {
        // Error loading
        lua_pop(L, 1);
        return false;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        // Error executing
        lua_pop(L, 1);
        return false;
    }

    return true;
}

String DiagnosticGenerator::severityToCpp(const String& severity)
{
    if (severity == "error")
        return "Severity::Error";
    if (severity == "warning")
        return "Severity::Warning";
    if (severity == "note")
        return "Severity::Note";
    if (severity == "fatal")
        return "Severity::Fatal";
    return "Severity::Error";
}

String DiagnosticGenerator::escapeForCpp(const String& str)
{
    StringBuilder sb;
    for (auto c : str)
    {
        switch (c)
        {
        case '"':
            sb << "\\\"";
            break;
        case '\\':
            sb << "\\\\";
            break;
        case '\n':
            sb << "\\n";
            break;
        case '\r':
            sb << "\\r";
            break;
        case '\t':
            sb << "\\t";
            break;
        default:
            sb.appendChar(c);
            break;
        }
    }
    return sb.produceString();
}

void DiagnosticGenerator::generateStruct(StringBuilder& sb, const DiagnosticDef& def)
{
    sb << "/// Parameters for the " << def.name << " diagnostic\n";
    sb << "struct " << def.name << "_Params\n";
    sb << "{\n";

    // Add parameter fields
    for (const auto& param : def.params)
    {
        sb << "    " << param.type << " " << param.name << ";\n";
    }

    // Add location fields for primary label
    if (def.primaryLabel.locName.getLength() > 0)
    {
        sb << "    SourceLoc " << def.primaryLabel.locName << ";\n";
    }

    // Add location fields for secondary labels
    for (const auto& label : def.secondaryLabels)
    {
        sb << "    SourceLoc " << label.locName << ";\n";
    }

    sb << "};\n\n";
}

void DiagnosticGenerator::generateBuilder(StringBuilder& sb, const DiagnosticDef& def)
{
    sb << "/// Build a RichDiagnostic for " << def.name << "\n";
    sb << "inline RichDiagnostic build_" << def.name << "(const " << def.name
       << "_Params& params)\n";
    sb << "{\n";
    sb << "    RichDiagnosticBuilder builder;\n";
    sb << "    builder.setCode(\"" << escapeForCpp(def.code) << "\");\n";
    sb << "    builder.setSeverity(" << severityToCpp(def.severity) << ");\n";

    // Build the main message (would need template substitution in real impl)
    sb << "    builder.setMessage(\"" << escapeForCpp(def.message) << "\");\n";

    // Add primary label
    if (def.primaryLabel.locName.getLength() > 0)
    {
        sb << "    builder.addPrimaryLabel(params." << def.primaryLabel.locName << ", \""
           << escapeForCpp(def.primaryLabel.message) << "\");\n";
    }

    // Add secondary labels
    for (const auto& label : def.secondaryLabels)
    {
        sb << "    builder.addSecondaryLabel(params." << label.locName << ", \""
           << escapeForCpp(label.message) << "\");\n";
    }

    // Add notes
    for (const auto& note : def.notes)
    {
        sb << "    builder.addNote(\"" << escapeForCpp(note) << "\");\n";
    }

    // Add helps
    for (const auto& help : def.helps)
    {
        sb << "    builder.addHelp(\"" << escapeForCpp(help) << "\");\n";
    }

    sb << "    return builder.build();\n";
    sb << "}\n\n";
}

String DiagnosticGenerator::generateHeader()
{
    StringBuilder sb;

    sb << "// GENERATED CODE - DO NOT EDIT\n";
    sb << "// Generated from Lua diagnostic definitions\n\n";
    sb << "#pragma once\n\n";
    sb << "#include \"slang-rich-diagnostic.h\"\n";
    sb << "#include \"slang-source-loc.h\"\n\n";
    sb << "namespace Slang\n";
    sb << "{\n\n";

    for (const auto& def : m_definitions)
    {
        generateStruct(sb, def);
        generateBuilder(sb, def);
    }

    sb << "} // namespace Slang\n";

    return sb.produceString();
}

String DiagnosticGenerator::generateSource()
{
    // For now, all code is in the header (inline functions)
    // This could be extended to generate .cpp files for larger implementations
    return String();
}

} // namespace fiddle