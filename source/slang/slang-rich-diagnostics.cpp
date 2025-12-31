#include "slang-rich-diagnostics.h"

#ifdef SLANG_PROTOTYPE_DIAGNOSTICS

#include "slang-ast-type.h"

//
#include "slang-rich-diagnostics.cpp.fiddle"

namespace Slang
{
namespace Diagnostics
{

// Generate member function implementations
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name) 
%     local params = lua_module.getUniqueParams(diagnostic)
GenericDiagnostic $(class_name)::toGenericDiagnostic() const
{
    GenericDiagnostic result;
    result.code = $(diagnostic.code);
    result.severity = $(lua_module.getSeverityEnum(diagnostic.severity));
    
    // Build message by interpolating parameters using StringBuilder
    StringBuilder messageBuilder;
%     for _, part in ipairs(diagnostic.message_parts) do
%         if part.type == "text" then
    messageBuilder << "$(part.content:gsub('"', '\\"'))";
%         elseif part.type == "interpolation" then
%             if part.param_type == "string" then
    messageBuilder << $(part.param_name);
%             elseif part.param_type == "name" then
    if ($(part.param_name))
    {
        messageBuilder << $(part.param_name)->text;
    }
    else
    {
        messageBuilder << "<unknown name>";
    }
%             elseif part.param_type == "type" then
    if ($(part.param_name))
    {
        messageBuilder << $(part.param_name)->toString();
    }
    else
    {
        messageBuilder << "<unknown type>";
    }
%             elseif part.param_type == "int" then
    messageBuilder << $(part.param_name);
%             end
%         end
%     end
    result.message = messageBuilder.produceString();
    
    // Set primary span
    result.primarySpan.location = $(diagnostic.primary_span.location);
    result.primarySpan.message = "$(diagnostic.primary_span.message)";
    
%     if diagnostic.secondary_spans then
    // Set secondary spans
%         for _, span in ipairs(diagnostic.secondary_spans) do
    {
        DiagnosticSpan span;
        span.location = $(span.location);
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
