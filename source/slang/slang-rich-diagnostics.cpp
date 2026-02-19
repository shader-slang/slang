#include "slang-rich-diagnostics.h"

#include "../compiler-core/slang-rich-diagnostics-render.h"
#include "slang-ast-modifier.h"
#include "slang-ast-type.h"

//
#include "slang-rich-diagnostics.cpp.fiddle"

namespace Slang
{
namespace Diagnostics
{

// Generate DiagnosticInfo entries for all rich diagnostics.
// These are needed for the warning suppression system (-Wno-xxx flags).
// The 'name' field uses lowerCamelCase which matches the convention expected
// by DiagnosticsLookup::findDiagnosticByName (which converts from kebab-case).
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local camel_name = lua_module.toLowerCamelCase(diagnostic.name)
%     local class_name = lua_module.toPascalCase(diagnostic.name)
%     -- Escape quotes and backslashes in the message
%     local escaped_message = diagnostic.message:gsub('\\', '\\\\'):gsub('"', '\\"')
const DiagnosticInfo $(camel_name)Info = {$(diagnostic.code), $(lua_module.getSeverityEnum(diagnostic.severity)), "$(camel_name)", "$(escaped_message)"};
const DiagnosticInfo* $(class_name)::getInfo() { return &$(camel_name)Info; }
% end

static const DiagnosticInfo* const kRichDiagnostics[] = {
% for _, diagnostic in ipairs(diagnostics) do
%     local camel_name = lua_module.toLowerCamelCase(diagnostic.name)
    &$(camel_name)Info,
% end
};

#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-rich-diagnostics.cpp.fiddle"
#endif // FIDDLE END

const DiagnosticInfo* const* getRichDiagnosticsInfo()
{
    return kRichDiagnostics;
}

Index getRichDiagnosticsInfoCount()
{
    return SLANG_COUNT_OF(kRichDiagnostics);
}

UnownedStringSlice nameToPrintableString(Name* name)
{
    return name ? name->text.getUnownedSlice() : UnownedStringSlice{"<unknown name>"};
}

String typeToPrintableString(Type* type)
{
    return type ? type->toString() : "<unknown type>";
}

String qualTypeToPrintableString(QualType type)
{
    return type.type ? type.type->toString() : "<unknown type>";
}

String modifierToPrintableString(Modifier* modifier)
{
    if (!modifier)
        return "<unknown modifier>";
    StringBuilder sb;
    if (modifier->keywordName && modifier->keywordName->text.getLength())
        sb << modifier->keywordName->text;
    if (auto hlslSemantic = as<HLSLSemantic>(modifier))
        sb << hlslSemantic->name.getContent();
    return sb.produceString();
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
%   -- Generate null check condition for required params
%   local generateNullCheck = function(required_params, var_mapping, variadic_struct)
%     var_mapping = var_mapping or {}
%     if not required_params or #required_params == 0 then
%       return nil
%     end
%     local checks = {}
%     for _, param_name in ipairs(required_params) do
%       local var_expr = var_mapping[param_name] or param_name
%       table.insert(checks, var_expr)
%     end
%     return table.concat(checks, " && ")
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
%           elseif ptype == "qualtype" then
          qualTypeToPrintableString($(base_expr))
%           elseif ptype == "decl" then
          nameToPrintableString($(base_expr)->getName())
%           elseif ptype == "modifier" then
          modifierToPrintableString($(base_expr))
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
%     -- Generate assertions for required params in main message and primary span
%     local all_asserted = {}
%     for _, param_name in ipairs(diagnostic.message_required_params or {}) do
%       if not all_asserted[param_name] then
%         all_asserted[param_name] = true
    SLANG_ASSERT($(param_name));
%       end
%     end
%     for _, param_name in ipairs(diagnostic.primary_span_required_params or {}) do
%       if not all_asserted[param_name] then
%         all_asserted[param_name] = true
    SLANG_ASSERT($(param_name));
%       end
%     end
    GenericDiagnostic result;
    result.code = $(diagnostic.code);
    result.severity = $(lua_module.getSeverityEnum(diagnostic.severity));

    result.message = $(buildMessage(diagnostic.message_parts));

%     if diagnostic.primary_span then
    // Set primary span
    result.primarySpan.range = SourceRange{$(lua_module.getLocationExpr(diagnostic.primary_span.location_name, diagnostic.primary_span.location_type))};
    result.primarySpan.message = $(buildMessage(diagnostic.primary_span.message_parts));
%     else
    // No primary span (locationless diagnostic)
    result.primarySpan.range = SourceRange{SourceLoc()};
    result.primarySpan.message = String();
%     end

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
%                 local null_check = generateNullCheck(vs.required_params, var_map, vs)
    for (const auto& $(item_var) : $(vs.list_name))
    {
%                 if null_check then
        if ($(null_check))
        {
%                 end
            DiagnosticSpan span;
            span.range = SourceRange{$(lua_module.getLocationExpr(var_map[span.location_name], span.location_type))};
            span.message = $(buildMessage(span.message_parts, var_map, vs));
            result.secondarySpans.add(span);
%                 if null_check then
        }
%                 end
    }
%             else
%                 local null_check = generateNullCheck(span.required_params, nil, nil)
%                 if null_check then
    if ($(null_check))
%                 end
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
%                 local null_check = generateNullCheck(vs.required_params, var_map, vs)
    for (const auto& $(item_var) : $(vs.list_name))
    {
%                 if null_check then
        if ($(null_check))
        {
%                 end
            DiagnosticNote note;
            note.span.range = SourceRange{$(lua_module.getLocationExpr(var_map[note.location_name], note.location_type))};
%                 if note.primary_span_message_parts then
            note.span.message = $(buildMessage(note.primary_span_message_parts, var_map, vs));
%                 end
            note.message = $(buildMessage(note.message_parts, var_map, vs));
%                 if note.spans and #note.spans > 0 then
            // Add additional spans to note
%                     for _, span in ipairs(note.spans) do
%                         local span_null_check = generateNullCheck(span.required_params, nil, nil)
%                         if span_null_check then
            if ($(span_null_check))
%                         end
            {
                DiagnosticSpan span;
                span.range = SourceRange{$(lua_module.getLocationExpr(span.location_name, span.location_type))};
                span.message = $(buildMessage(span.message_parts));
                note.secondarySpans.add(span);
            }
%                     end
%                 end
            result.notes.add(note);
%                 if null_check then
        }
%                 end
    }
%             else
%                 local null_check = generateNullCheck(note.required_params, nil, nil)
%                 -- For notes with untyped locations (plain SourceLoc), check validity
%                 local validity_check = nil
%                 if not note.location_type then
%                     validity_check = note.location_name .. ".isValid()"
%                 end
%                 -- Combine checks
%                 local combined_check = nil
%                 if validity_check and null_check then
%                     combined_check = validity_check .. " && " .. null_check
%                 elseif validity_check then
%                     combined_check = validity_check
%                 elseif null_check then
%                     combined_check = null_check
%                 end
%                 if combined_check then
    if ($(combined_check))
%                 end
    {
        DiagnosticNote note;
        note.span.range = SourceRange{$(lua_module.getLocationExpr(note.location_name, note.location_type))};
%                 if note.primary_span_message_parts then
        note.span.message = $(buildMessage(note.primary_span_message_parts));
%                 end
        note.message = $(buildMessage(note.message_parts));
%                 if note.spans and #note.spans > 0 then
        // Add additional spans to note
%                     for _, span in ipairs(note.spans) do
%                         local span_null_check = generateNullCheck(span.required_params, nil, nil)
%                         if span_null_check then
        if ($(span_null_check))
%                         end
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
#define FIDDLE_GENERATED_OUTPUT_ID 1
#include "slang-rich-diagnostics.cpp.fiddle"
#endif // FIDDLE END

} // namespace Diagnostics
} // namespace Slang
