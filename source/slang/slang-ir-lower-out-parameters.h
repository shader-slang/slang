#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink);

} // namespace Slang
