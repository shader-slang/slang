// slang-ir-operator-shift-overflow.h
#pragma once

#include "slang/slang-compiler-options.h"

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void checkForOperatorShiftOverflow(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
