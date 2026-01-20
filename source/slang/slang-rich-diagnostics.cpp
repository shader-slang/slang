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

%   local getParamType = function(param_name)
%     for _, param in ipairs(diagnostic.params) do
%       if param.name == param_name then
%         return param.type
%       end
%     end
%     for _, loc in ipairs(diagnostic.locations) do
%       if loc.name == param_name and loc.type then
%         return loc.type
%       end
%     end
%     return "string"
%   end
%
%   local buildMessage = function(parts, var_mapping)
%     var_mapping = var_mapping or {}
%     local emitPart = function(part)
%       if part.type == "text" then
          "$(part.content:gsub('"', '\\"'))"
%       elseif part.type == "interpolation" then
%         if part.member_name then
%           -- Member access: ~param.member or ~param:Type.member
%           local ptype = getParamType(part.param_name)
%           local base_expr = var_mapping[part.param_name] or part.param_name
%           local cpp_expr, result_type = lua_module.resolveMemberAccess(ptype, part.member_name, base_expr)
%           if result_type == "name" then
          nameToPrintableString($(cpp_expr))
%           elseif result_type == "type" then
          typeToPrintableString($(cpp_expr))
%           else
          $(cpp_expr)
%           end
%         else
%           -- Direct parameter reference
%           local ptype = getParamType(part.param_name)
%           local base_expr = var_mapping[part.param_name] or part.param_name
%           if ptype == "name" then
          nameToPrintableString($(base_expr))
%           elseif ptype == "type" then
          typeToPrintableString($(base_expr))
%           elseif ptype == "decl" then
          nameToPrintableString($(base_expr)->getName())
%           elseif ptype == "expr" or ptype == "stmt" or ptype == "val" then
          $(base_expr)
%           else
          $(base_expr)
%           end
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
    result.primarySpan.range = SourceRange{$(lua_module.getLocationExpr(diagnostic.primary_span.location_name, diagnostic.primary_span.location_type))};
    result.primarySpan.message = $(buildMessage(diagnostic.primary_span.message_parts));
    
%     if diagnostic.secondary_spans then
    // Set secondary spans
%         for _, span in ipairs(diagnostic.secondary_spans) do
%             if span.variadic then
%                 local item_var = span.location_name .. "_item"
%                 local var_map = {[span.location_name] = item_var}
    for (auto $(item_var) : $(span.location_name)) {
        DiagnosticSpan span;
        span.range = SourceRange{$(lua_module.getLocationExpr(item_var, span.location_type))};
        span.message = $(buildMessage(span.message_parts, var_map));
        result.secondarySpans.add(span);
    }
%             else
    {
        DiagnosticSpan span;
        span.range = SourceRange{$(lua_module.getLocationExpr(span.location_name, span.location_type))};
        span.message = $(buildMessage(span.message_parts));
        result.secondarySpans.add(span);
    }
%             end
%         end
%     end

%     if diagnostic.notes then
    // Set notes
%         for _, note in ipairs(diagnostic.notes) do
%             if note.variadic then
%                 local item_var = note.location_name .. "_item"
%                 local var_map = {[note.location_name] = item_var}
    for (auto $(item_var) : $(note.location_name)) {
        DiagnosticNote note;
        note.span.range = SourceRange{$(lua_module.getLocationExpr(item_var, note.location_type))};
        note.message = $(buildMessage(note.message_parts, var_map));
        result.notes.add(note);
    }
%             else
    {
        DiagnosticNote note;
        note.span.range = SourceRange{$(lua_module.getLocationExpr(note.location_name, note.location_type))};
        note.message = $(buildMessage(note.message_parts));
        result.notes.add(note);
    }
%             end
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
