#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Return a wrapper for entry point `func` that returns its `out`/`inout` parameters in a struct.
/// The wrapper takes over `func`'s entry-point identity, so callers take the entry-point
/// decoration from the returned function. `func` is inlined into the wrapper and deleted unless it
/// has other users, in which case it remains as an ordinary function. Returns `func` itself when
/// no lowering is needed.
IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct);

} // namespace Slang
