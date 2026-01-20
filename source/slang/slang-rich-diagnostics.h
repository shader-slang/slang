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
%     for _, param in ipairs(diagnostic.params) do
%         local type = lua_module.getCppType(param.type)
%         if param.variadic then
    List<$(type)> $(param.name) = {};
%         else
%             local initializer = (type:sub(-1) == "*") and "nullptr" or type .. "{}"
    $(type) $(param.name) = $(initializer);
%         end
%     end

%     for _, loc in ipairs(diagnostic.locations) do
%         if loc.type then
%             local loc_cpp_type = lua_module.getCppType(loc.type)
%             if loc.variadic then
    List<$(loc_cpp_type)> $(loc.name) = {};
%             else
%                 local loc_initializer = (loc_cpp_type:sub(-1) == "*") and "nullptr" or loc_cpp_type .. "{}"
    $(loc_cpp_type) $(loc.name) = $(loc_initializer);
%             end
%         else
%             if loc.variadic then
    List<SourceLoc> $(loc.name) = {};
%             else
    SourceLoc $(loc.name) = SourceLoc{};
%             end
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
