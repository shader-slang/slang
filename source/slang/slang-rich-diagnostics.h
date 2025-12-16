#pragma once

#ifdef SLANG_PROTOTYPE_DIAGNOSTIC_STYLE

#include "slang-diagnostic.h"
#include "slang-source-loc.h"
#include "slang-string.h"

//
#include "slang-rich-diagnostics.h.fiddle"

namespace Slang {

// Forward declarations for rich diagnostic types
struct RichDiagnostic;

// Generate parameter and builder structures for all diagnostics
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
$(lua_module.generateAllDiagnostics())
#else // FIDDLE OUTPUT:
// Fiddle output goes here.
#endif // FIDDLE END

// Generate factory functions
#if 0 // FIDDLE TEMPLATE:
% local lua_module = require("source/slang/slang-rich-diagnostics.h.lua")
$(lua_module.generateFactoryFunctions())
#else // FIDDLE OUTPUT:
// Fiddle output goes here.
#endif // FIDDLE END

} // namespace Slang

#endif // SLANG_PROTOTYPE_DIAGNOSTIC_STYLE