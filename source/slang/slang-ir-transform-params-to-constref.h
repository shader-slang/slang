// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Transform by-value aggregate `in` parameters of ordinary (non-entry-point) functions into
// `borrow in` (pointer) parameters so callers can pass an address instead of copying the value.
//
// When `forwardEntryPointUniformAddress` is set (CUDA only), a call *argument* that is an
// entry-point by-value uniform aggregate parameter is forwarded by address (`&param`) rather than
// spilled through a temporary copy - the #11774 slowdown. Only the callee's parameter is rewritten
// to `borrow in`; the entry-point uniform stays a plain by-value kernel parameter, appearing here
// only as a forwarded argument.
SlangResult transformParamsToConstRef(
    IRModule* module,
    DiagnosticSink* sink,
    bool forwardEntryPointUniformAddress = false);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
