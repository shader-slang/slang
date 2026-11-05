// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Transform by-value aggregate `in` parameters of ordinary (non-entry-point) functions into
// `borrow in` parameters, so that callers pass an address instead of a copy of the value.
//
// On CUDA targets (determined from `targetReq`), an entry point's uniform parameters of those types
// are also passed to such callees by address. The emitted kernel still declares them by value.
SlangResult transformByValueParamsToBorrows(
    IRModule* module,
    TargetRequest* targetReq,
    DiagnosticSink* sink);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
