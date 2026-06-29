// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink);

/// Rewrite an entry point's varying `in` parameters to `borrow in` (pointer-backed) parameters.
///
/// When `transformCudaResourceArrayUniforms` is true (CUDA targets), this also rewrites a
/// *uniform* parameter that is a runtime-indexed fixed-size array of resources to `borrow in`.
/// Such a uniform is otherwise passed by value in the un-indexable CUDA `.param` space, so a
/// runtime index (`arr[indices[tid]]`) lowers to a serial `ld.param` chain; passing it by
/// reference makes the index an ordinary dynamically-addressable load. Other entry-point uniforms
/// are left by value.
SlangResult translateEntryPointInParamToBorrow(
    IRModule* module,
    DiagnosticSink* sink,
    bool transformCudaResourceArrayUniforms);

} // namespace Slang
