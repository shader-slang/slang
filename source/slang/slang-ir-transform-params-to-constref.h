// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
struct CodeGenContext;

// Supplying cudaContext enables internal CUDA value-argument optimization before
// composites are lowered to borrowed pointers. Other targets keep the existing policy.
SlangResult transformParamsToConstRef(
    IRModule* module,
    DiagnosticSink* sink,
    CodeGenContext* cudaContext = nullptr);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
