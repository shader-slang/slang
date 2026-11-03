// slang-ir-check-printf-args.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

/// Report every `printf` argument that is or contains an opaque value, such as a resource.
///
/// The front end rejects such an argument when its type is known at the `printf` call. A resource
/// can still reach `printf` through a generic: when `void log<T>(T a)` calls `printf("%u\n", a)`
/// and is specialized with `T = ConstantBuffer<uint>`, the argument type is only known after
/// specialization. We check the specialized module, before any target legalization, so that every
/// target reports the argument instead of passing a handle to its print call.
void checkPrintfArgumentTypes(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
