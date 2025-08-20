// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{
// Main entry point for the pass
bool specializeDynamicInsts(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
