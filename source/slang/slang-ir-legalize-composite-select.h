#pragma once

#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

void legalizeNonVectorCompositeSelect(IRModule* module);
} // namespace Slang
