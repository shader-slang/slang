#pragma once

#ifdef SLANG_PROTOTYPE_DIAGNOSTICS

#include "slang-diagnostic.h"
#include "slang-source-loc.h"
#include "slang-string.h"

//
#include "slang-rich-diagnostics.h.fiddle"

namespace Slang
{

// Generic diagnostic representation for layout rendering
struct DiagnosticSpan
{
    SourceLoc location;
    String message;
};

struct GenericDiagnostic
{
    int code;
    String severity;
    String message;
    DiagnosticSpan primarySpan;
    List<DiagnosticSpan> secondarySpans;
};

namespace Diagnostics
{

// Generate parameter structures for all diagnostics
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name) .. "Params"
%     local params = lua_module.getUniqueParams(diagnostic)
struct $(class_name)
{
%     for _, param in ipairs(params) do
    $(param.cpp_type) $(param.name);
%     end
    SourceLoc $(diagnostic.primary_span.location);
%     if diagnostic.secondary_spans then
%         for _, span in ipairs(diagnostic.secondary_spans) do
    SourceLoc $(span.location);
%         end
%     end
};

% end

// Generate conversion functions to GenericDiagnostic
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name) .. "Params"
%     local function_name = lua_module.toSnakeCase(diagnostic.name)
GenericDiagnostic convertTo$(class_name:gsub("Params", ""))(const $(class_name)& params);
% end

#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-rich-diagnostics.h.fiddle"
#endif // FIDDLE END

} // namespace Diagnostics
} // namespace Slang

#endif // SLANG_PROTOTYPE_DIAGNOSTICS