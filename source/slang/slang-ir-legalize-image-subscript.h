#pragma once

#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

void legalizeImageSubscript(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
