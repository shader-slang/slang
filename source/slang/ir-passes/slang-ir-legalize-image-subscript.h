#pragma once

#include "slang/ir/slang-ir.h"
#include "slang/slang-compiler.h"

namespace Slang
{
class DiagnosticSink;

void legalizeImageSubscript(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
