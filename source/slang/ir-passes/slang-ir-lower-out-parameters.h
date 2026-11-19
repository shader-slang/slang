#pragma once

#include "slang/ir/slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct);

} // namespace Slang
