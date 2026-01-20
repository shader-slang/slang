#pragma once

#include "../compiler-core/slang-diagnostic-sink.h"
#include "../compiler-core/slang-rich-diagnostics-render.h"
#include "../core/slang-basic.h"


//
#include "slang-rich-diagnostics.h.fiddle"

namespace Slang
{

class Type;
class Decl;
class Expr;
class Stmt;
class Val;
class Name;

namespace Diagnostics
{

// Generate parameter structures for all diagnostics
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
% local diagnostics = lua_module.getDiagnostics()
% for _, diagnostic in ipairs(diagnostics) do
%     local class_name = lua_module.toPascalCase(diagnostic.name)
struct $(class_name)
{
%     -- Direct parameters (non-variadic or shared)
%     for _, param in ipairs(diagnostic.params) do
%         local type = lua_module.getCppType(param.type)
%         local initializer = (type:sub(-1) == "*") and "nullptr" or type .. "{}"
    $(type) $(param.name) = $(initializer);
%     end

%     -- Direct locations (non-variadic or shared)
%     for _, loc in ipairs(diagnostic.locations) do
%         if loc.type then
%             local loc_cpp_type = lua_module.getCppType(loc.type)
%             local loc_initializer = (loc_cpp_type:sub(-1) == "*") and "nullptr" or loc_cpp_type .. "{}"
    $(loc_cpp_type) $(loc.name) = $(loc_initializer);
%         else
    SourceLoc $(loc.name) = SourceLoc{};
%         end
%     end

%     -- Nested structs for variadic spans/notes (AoS pattern)
%     if diagnostic.variadic_structs then
%         for _, vs in ipairs(diagnostic.variadic_structs) do
    struct $(vs.struct_name)
    {
%             for _, loc in ipairs(vs.locations) do
%                 if loc.type then
%                     local loc_cpp_type = lua_module.getCppType(loc.type)
%                     local loc_initializer = (loc_cpp_type:sub(-1) == "*") and "nullptr" or loc_cpp_type .. "{}"
        $(loc_cpp_type) $(loc.name) = $(loc_initializer);
%                 else
        SourceLoc $(loc.name) = SourceLoc{};
%                 end
%             end
%             for _, param in ipairs(vs.params) do
%                 local type = lua_module.getCppType(param.type)
%                 local initializer = (type:sub(-1) == "*") and "nullptr" or type .. "{}"
        $(type) $(param.name) = $(initializer);
%             end
    };
    List<$(vs.struct_name)> $(vs.list_name) = {};

%         end
%     end
    GenericDiagnostic toGenericDiagnostic() const;
};

% end

#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-rich-diagnostics.h.fiddle"
#endif // FIDDLE END

} // namespace Diagnostics
} // namespace Slang
