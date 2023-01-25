// slang-ir-check-differentiability.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;
struct SharedIRBuilder;

// Check all auto diff usages are valid.
void checkAutoDiffUsages(SharedIRBuilder* sharedBuilder, IRModule* module, DiagnosticSink* sink);

} // namespace Slang
