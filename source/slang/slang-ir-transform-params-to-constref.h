// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Transform by-value aggregate `in` parameters of ordinary (non-entry-point) functions into
// `borrow in` (pointer) parameters so callers can pass an address instead of copying the value.
//
// When `forwardEntryPointUniformAddress` is set (CUDA only), a call argument that is itself an
// entry-point by-value uniform aggregate parameter is forwarded by address (`&param`) instead of
// being spilled through a temporary copy first. This complements the CUDA emitter marking such
// parameters `__grid_constant__ const`, so a large kernel uniform is read in place from constant
// grid memory rather than copied into every thread's local stack frame.
SlangResult transformParamsToConstRef(
    IRModule* module,
    DiagnosticSink* sink,
    bool forwardEntryPointUniformAddress = false);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
