// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "core/slang-func-ptr.h"
#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Narrow partially consumed internal struct parameters while retaining value snapshots.
// Only expose fields whose types the caller accepts as independent value parameters.
SlangResult transformAggregateParamsToFields(
    IRModule* module,
    DiagnosticSink* sink,
    const Func<bool, IRInst*>& isFieldTypeSupported);

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
