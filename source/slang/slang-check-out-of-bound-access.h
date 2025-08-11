// slang-check-out-of-bound-access.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

void checkForOutOfBoundAccess(IRModule* module, DiagnosticSink* sink);
} // namespace Slang