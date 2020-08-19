// slang-ir-hoist-local-types.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Hoist local types to global scope if possible.
/// Some transformation passes may leave behind local type definitons that
/// can be hoisted to global scope. This pass examines all local type defintions
// and try to hoist them to global scope if the definition is no longer dependent on
// the local context.
void hoistLocalTypes(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
