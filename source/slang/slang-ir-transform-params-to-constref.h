// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Transform by-value aggregate `in` parameters of ordinary (non-entry-point) functions into
// `borrow in` (pointer) parameters so callers can pass an address instead of copying the value.
//
// On CUDA targets (determined from `targetReq`), an entry-point by-value uniform aggregate
// parameter is also passed to such callees by address instead of through a per-thread copy
// (#11774). The kernel signature is unchanged: the parameter is still emitted as a by-value
// argument.
SlangResult transformParamsToConstRef(
    IRModule* module,
    TargetRequest* targetReq,
    DiagnosticSink* sink);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
