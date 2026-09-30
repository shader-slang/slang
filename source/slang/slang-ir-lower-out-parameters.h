#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Replace entry point `func` with a wrapper that returns `func`'s result and its `out`/`inout`
/// parameters in a struct, and return the wrapper. Without `alwaysUseReturnStruct`, a `func` with
/// no pure `out` parameter is returned unchanged. The wrapper takes over all of `func`'s
/// decorations and the `IREntryPointParamDecoration`s of its hoisted uniforms; `func` is then
/// inlined into the wrapper and deleted, so callers must not use `func` or its decorations
/// afterwards. Every other call to `func` must already have been redirected by
/// fixEntryPointCallsites.
IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct);

} // namespace Slang
