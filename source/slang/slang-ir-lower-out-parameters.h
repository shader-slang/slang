#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Replace entry point `func` with a wrapper that returns `func`'s result and its `out`/`inout`
/// parameters, and return the wrapper. With `alwaysUseReturnStruct` the wrapper always returns a
/// struct; without it, a single returned value is returned directly, and a `func` with no pure
/// `out` parameter is returned unchanged. The wrapper takes over all of `func`'s decorations and
/// the `IREntryPointParamDecoration`s of its hoisted uniforms; `func` is then inlined into the
/// wrapper and deleted, so callers must not use `func` or its decorations afterwards.
///
/// `func` must have no uses other than those `IREntryPointParamDecoration`s: fixEntryPointCallsites
/// has already redirected every call to a separate ordinary-function copy.
IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct);

} // namespace Slang
