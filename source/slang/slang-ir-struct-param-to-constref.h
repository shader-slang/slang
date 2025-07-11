// slang-ir-struct-param-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

// Transform struct and array parameters to use ConstRef<T> instead of pass-by-value
// for better performance in CUDA/C++ code generation.
//
// This pass transforms functions like:
//   void f(S structVal) { ... }
//   f(s);
//
// Into:
//   void f(ConstRef<S> structVal) { ... }
//   f(&s);
//
// Within function bodies, it transforms:
//   fieldExtract(param, field) -> load(fieldAddress(param, field))
//   getElement(param, index) -> load(getElementPtr(param, index))
SlangResult transformStructParamsToConstRef(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
