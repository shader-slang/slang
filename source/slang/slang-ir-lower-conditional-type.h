// slang-ir-lower-conditional-type.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

/// Lower `Conditional<T, hasValue>` intrinsic types into concrete representations.
/// Conditional<T, true> is replaced with T directly.
/// Conditional<T, false> is replaced with a shared zero-field empty struct.
void lowerConditionalType(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
