#pragma once

#include "../compiler-core/slang-diagnostic-sink.h"
#include "../compiler-core/slang-rich-diagnostics-render.h"
#include "../core/slang-basic.h"


//
#include "slang-rich-diagnostics.h.fiddle"

namespace Slang
{

class Type;

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
%         local initializer = (type:sub(-1) == "*") and "nullptr" or type .. "{}"
    $(type) $(param.name) = $(initializer);
%     end

%     for _, loc in ipairs(diagnostic.locations) do
    SourceLoc $(loc.name) = SourceLoc{};
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
