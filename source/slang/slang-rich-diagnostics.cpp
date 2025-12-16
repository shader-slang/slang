#include "slang-rich-diagnostics.h"

#ifdef SLANG_PROTOTYPE_DIAGNOSTICS

//
#include "slang-rich-diagnostics.cpp.fiddle"

namespace Slang
{
namespace Diagnostics
{

// Generate conversion function implementations
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name) .. "Params"
%     local diagnostic_class = class_name:gsub("Params", "")
%     local params = lua_module.getUniqueParams(diagnostic)
GenericDiagnostic convertTo$(diagnostic_class)(const $(class_name)& params)
{
    GenericDiagnostic result;
    result.code = $(diagnostic.code);
    result.severity = "$(diagnostic.severity)";
    
    // Build message by interpolating parameters
    String message = "";
%     for _, part in ipairs(diagnostic.message_parts) do
%         if part.type == "text" then
    message = message + "$(part.content:gsub('"', '\\"'))";
%         elseif part.type == "interpolation" then
%             if part.param_type == "string" then
    message = message + params.$(part.param_name);
%             elseif part.param_type == "type" then
    message = message + params.$(part.param_name)->toString();
%             elseif part.param_type == "int" then
    message = message + String(params.$(part.param_name));
%             end
%         end
%     end
    result.message = message;
    
    // Set primary span
    result.primarySpan.location = params.$(diagnostic.primary_span.location);
    result.primarySpan.message = "$(diagnostic.primary_span.message)";
    
%     if diagnostic.secondary_spans then
    // Set secondary spans
%         for _, span in ipairs(diagnostic.secondary_spans) do
    {
        DiagnosticSpan span;
        span.location = params.$(span.location);
        span.message = "$(span.message)";
        result.secondarySpans.add(span);
    }
%         end
%     end
    
    return result;
}

% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-rich-diagnostics.cpp.fiddle"
#endif // FIDDLE END

} // namespace Diagnostics
} // namespace Slang

#endif // SLANG_PROTOTYPE_DIAGNOSTICS