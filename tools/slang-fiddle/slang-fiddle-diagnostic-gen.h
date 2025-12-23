// slang-fiddle-diagnostic-gen.h
// Generator for C++ diagnostic structures from Lua definitions

#ifndef SLANG_FIDDLE_DIAGNOSTIC_GEN_H
#define SLANG_FIDDLE_DIAGNOSTIC_GEN_H

#include "compiler-core/slang-diagnostic-sink.h"
#include "core/slang-basic.h"

// Forward declare lua_State from Lua headers
struct lua_State;

namespace fiddle
{
using namespace Slang;

/// Represents a parameter in a diagnostic definition
struct DiagnosticParam
{
    String name;
    String type;
};

/// Represents a label (primary or secondary) in a diagnostic
struct DiagnosticLabelDef
{
    String locName; // Name of the location parameter
    String message; // Message template for the label
    bool isPrimary = false;
};

/// Represents a complete diagnostic definition from Lua
struct DiagnosticDef
{
    String name;     // Diagnostic identifier (e.g., "argument_type_mismatch")
    String code;     // Error code (e.g., "E30019")
    String severity; // "error", "warning", "note"
    String flag;     // Compiler flag name
    String message;  // Main message template

    List<DiagnosticParam> params;
    DiagnosticLabelDef primaryLabel;
    List<DiagnosticLabelDef> secondaryLabels;
    List<String> notes;
    List<String> helps;
};

/// Generator that reads Lua diagnostic definitions and produces C++ code
class DiagnosticGenerator
{
public:
    DiagnosticGenerator(DiagnosticSink* sink)
        : m_sink(sink)
    {
    }

    /// Load diagnostic definitions from a Lua file
    bool loadFromLuaFile(const String& filePath);

    /// Generate C++ header with diagnostic structures
    String generateHeader();

    /// Generate C++ source with diagnostic builders
    String generateSource();

    /// Get the loaded diagnostic definitions
    const List<DiagnosticDef>& getDefinitions() const { return m_definitions; }

private:
    DiagnosticSink* m_sink;
    List<DiagnosticDef> m_definitions;

    /// Parse a diagnostic definition from the Lua state
    bool parseDiagnosticDef(lua_State* L, DiagnosticDef& outDef);

    /// Generate C++ struct for a diagnostic
    void generateStruct(StringBuilder& sb, const DiagnosticDef& def);

    /// Generate builder function for a diagnostic
    void generateBuilder(StringBuilder& sb, const DiagnosticDef& def);

    /// Convert severity string to C++ enum
    String severityToCpp(const String& severity);

    /// Escape a string for C++ string literal
    String escapeForCpp(const String& str);
};

} // namespace fiddle

#endif // SLANG_FIDDLE_DIAGNOSTIC_GEN_H