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
%
%   -- Build a lookup table for variadic struct info
%   local variadic_struct_by_name = {}
%   if diagnostic.variadic_structs then
%     for _, vs in ipairs(diagnostic.variadic_structs) do
%       variadic_struct_by_name[vs.struct_name] = vs
%     end
%   end
%
%   -- getParamType: looks in direct params, direct locations, and variadic struct members
%   local getParamType = function(param_name, variadic_struct)
%     -- First check direct params
%     for _, param in ipairs(diagnostic.params) do
%       if param.name == param_name then
%         return param.type
%       end
%     end
%     -- Check direct locations
%     for _, loc in ipairs(diagnostic.locations) do
%       if loc.name == param_name and loc.type then
%         return loc.type
%       end
%     end
%     -- Check variadic struct members if provided
%     if variadic_struct then
%       for _, param in ipairs(variadic_struct.params) do
%         if param.name == param_name then
%           return param.type
%         end
%       end
%       for _, loc in ipairs(variadic_struct.locations) do
%         if loc.name == param_name and loc.type then
%           return loc.type
%         end
%       end
%     end
%     return "string"
%   end
%
%   local buildMessage = function(parts, var_mapping, variadic_struct)
%     var_mapping = var_mapping or {}
%     local emitPart = function(part)
%       if part.type == "text" then
          "$(part.content:gsub('"', '\\"'))"
%       elseif part.type == "interpolation" then
%         if part.member_name then
%           -- Member access: ~param.member or ~param:Type.member
%           local ptype = getParamType(part.param_name, variadic_struct)
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
%           local ptype = getParamType(part.param_name, variadic_struct)
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
GenericDiagnostic $(class_name)::toGenericDiagnostic() const
{
    GenericDiagnostic result;
    result.code = $(diagnostic.code);
    result.severity = $(lua_module.getSeverityEnum(diagnostic.severity));

    result.message = $(buildMessage(diagnostic.message_parts));

    // Set primary span
    result.primarySpan.range = SourceRange{$(lua_module.getLocationExpr(diagnostic.primary_span.location_name, diagnostic.primary_span.location_type))};
    result.primarySpan.message = $(buildMessage(diagnostic.primary_span.message_parts));

%     if diagnostic.secondary_spans and #diagnostic.secondary_spans > 0 then
    // Set secondary spans
%         for _, span in ipairs(diagnostic.secondary_spans) do
%             if span.variadic and span.struct_name then
%                 -- AoS iteration: loop over the struct list
%                 local vs = variadic_struct_by_name[span.struct_name]
%                 local item_var = "item"
%                 -- Build variable mapping for struct members
%                 local var_map = {}
%                 for _, loc in ipairs(vs.locations) do
%                     var_map[loc.name] = item_var .. "." .. loc.name
%                 end
%                 for _, param in ipairs(vs.params) do
%                     var_map[param.name] = item_var .. "." .. param.name
%                 end
    for (const auto& $(item_var) : $(vs.list_name))
    {
        DiagnosticSpan span;
        span.range = SourceRange{$(lua_module.getLocationExpr(var_map[span.location_name], span.location_type))};
        span.message = $(buildMessage(span.message_parts, var_map, vs));
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

%     if diagnostic.notes and #diagnostic.notes > 0 then
    // Set notes
%         for _, note in ipairs(diagnostic.notes) do
%             if note.variadic and note.struct_name then
%                 -- AoS iteration: loop over the struct list
%                 local vs = variadic_struct_by_name[note.struct_name]
%                 local item_var = "item"
%                 -- Build variable mapping for struct members
%                 local var_map = {}
%                 for _, loc in ipairs(vs.locations) do
%                     var_map[loc.name] = item_var .. "." .. loc.name
%                 end
%                 for _, param in ipairs(vs.params) do
%                     var_map[param.name] = item_var .. "." .. param.name
%                 end
    for (const auto& $(item_var) : $(vs.list_name))
    {
        DiagnosticNote note;
        note.span.range = SourceRange{$(lua_module.getLocationExpr(var_map[note.location_name], note.location_type))};
        note.message = $(buildMessage(note.message_parts, var_map, vs));
%                 if note.spans and #note.spans > 0 then
        // Add additional spans to note
%                     for _, span in ipairs(note.spans) do
        {
            DiagnosticSpan span;
            span.range = SourceRange{$(lua_module.getLocationExpr(span.location_name, span.location_type))};
            span.message = $(buildMessage(span.message_parts));
            note.secondarySpans.add(span);
        }
%                     end
%                 end
        result.notes.add(note);
    }
%             else
    {
        DiagnosticNote note;
        note.span.range = SourceRange{$(lua_module.getLocationExpr(note.location_name, note.location_type))};
        note.message = $(buildMessage(note.message_parts));
%                 if note.spans and #note.spans > 0 then
        // Add additional spans to note
%                     for _, span in ipairs(note.spans) do
        {
            DiagnosticSpan span;
            span.range = SourceRange{$(lua_module.getLocationExpr(span.location_name, span.location_type))};
            span.message = $(buildMessage(span.message_parts));
            note.secondarySpans.add(span);
        }
%                     end
%                 end
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
