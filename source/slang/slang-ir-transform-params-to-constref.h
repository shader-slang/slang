// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetRequest;

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

/// WGSL variant of `transformParamsToConstRef`: rewrite `in` struct and sized-array
/// parameters whose natural size is larger than `minSizeInBytes` to `ptr<function, T>`.
/// Call sites pass a function-scope temporary, or forward an incoming pointer parameter.
SlangResult transformLargeParamsToConstRefForWGSL(
    IRModule* module,
    TargetRequest* targetRequest,
    IRIntegerValue minSizeInBytes,
    DiagnosticSink* sink);

} // namespace Slang
