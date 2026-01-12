#include "slang-rich-diagnostics.h"

#include "../compiler-core/slang-rich-diagnostics-render.h"
#include "slang-ast-type.h"

//
#include "slang-rich-diagnostics.cpp.fiddle"

namespace Slang
{
namespace Diagnostics
{

UnownedStringSlice nameToPrintableString(Name* name)
{
    return name ? name->text.getUnownedSlice() : UnownedStringSlice{"<unknown name>"};
}

UnownedStringSlice typeToPrintableString(Type* type)
{
    return type ? type->toString().getUnownedSlice() : UnownedStringSlice{"<unknown type>"};
}

// Generate member function implementations
//
// This section generates a function which goes from this specific diagnostic struct, for example:
//
//     struct FunctionRedefinition
//     {
//         Name* name = nullptr;
//
//         SourceLoc function_location = SourceLoc{};
//         SourceLoc original_location = SourceLoc{};
//
//         GenericDiagnostic toGenericDiagnostic() const;
//     };
//
// to GenericDiagnostic, with a function that looks something like this:
//
//     GenericDiagnostic FunctionRedefinition::toGenericDiagnostic() const
//     {
//         GenericDiagnostic result;
//         result.code = 30201;
//         result.severity = Severity::Error;
//
//         result.message =
//             (StringBuilder{} << "function '"
//                              << nameToPrintableString(name)
//                              << "' already has a body").produceString();
//
//         // Set primary span
//         result.primarySpan.loc = function_location;
//         result.primarySpan.message = "redeclared here";
//
//         // Set notes
//         {
//             DiagnosticNote note;
//             note.span.loc = original_location;
//             note.message = (StringBuilder{} << "see previous definition of '"
//                                             << nameToPrintableString(name) << "'")
//                                .produceString();
//             result.notes.add(note);
//         }
//
//         return result;
//     }
//
//
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name) 
GenericDiagnostic $(class_name)::toGenericDiagnostic() const
{
    GenericDiagnostic result;
    result.code = $(diagnostic.code);
    result.severity = $(lua_module.getSeverityEnum(diagnostic.severity));

%   local buildMessage = function(parts)
%     local emitPart = function(part)
%       if part.type == "text" then
          "$(part.content:gsub('"', '\\"'))"
%       elseif part.type == "interpolation" then
%         if part.param_type == "name" then
          nameToPrintableString($(part.param_name))
%         elseif part.param_type == "type" then
          typeToPrintableString($(part.param_name))
%         else
          $(part.param_name)
%         end
%       end
%     end
%
%     if #parts == 1 then
        $(emitPart(parts[1]))
%     else
        (StringBuilder{}
%       for _, part in ipairs(parts) do
          << $(emitPart(part))
%       end
        ).produceString()
%     end
%   end    
    result.message = $(buildMessage(diagnostic.message_parts));
    
    // Set primary span
    result.primarySpan.range = SourceRange{$(diagnostic.primary_span.location)};
    result.primarySpan.message = $(buildMessage(diagnostic.primary_span.message_parts));
    
%     if diagnostic.secondary_spans then
    // Set secondary spans
%         for _, span in ipairs(diagnostic.secondary_spans) do
    {
        DiagnosticSpan span;
        span.range = SourceRange{$(span.location)};
        span.message = $(buildMessage(span.message_parts));
        result.secondarySpans.add(span);
    }
%         end
%     end

%     if diagnostic.notes then
    // Set notes
%         for _, note in ipairs(diagnostic.notes) do
    {
        DiagnosticNote note;
        note.span.range = SourceRange{$(note.location)};
        note.message = $(buildMessage(note.message_parts));
        result.notes.add(note);
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
