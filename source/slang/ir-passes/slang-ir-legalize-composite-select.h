#pragma once

#include "slang/ir/slang-ir.h"
#include "slang/slang-compiler.h"

namespace Slang
{
class DiagnosticSink;

void legalizeNonVectorCompositeSelect(IRModule* module);
} // namespace Slang
