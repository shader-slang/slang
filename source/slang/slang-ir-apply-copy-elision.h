// source\slang\slang-ir-apply-copy-elision.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

SlangResult applyCopyElision(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
