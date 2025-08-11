// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
