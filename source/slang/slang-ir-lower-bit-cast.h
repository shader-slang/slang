// slang-ir-lower-bit-cast.h
#pragma once

// This file defines an IR pass that lowers a BitCast<T>(U) operation, where T and U are struct
// types, into a series of bit-cast operations on basic-typed elements.

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

void lowerBitCast(IRModule* module, TargetProgram* targetReq, DiagnosticSink* sink);

/// Report an error for every `BitCast` from plain data to a type that is or contains a
/// `DescriptorHandle<T>`.
///
/// Depending on the target and resource kind, a handle is a `uint2` heap index, a `uint64_t`, or
/// the native resource, so `lowerBitCast` has no byte layout to rebuild one from; the
/// `DescriptorHandle` constructor is the defined way to make one. The check runs before AnyValue
/// marshalling and byte-address-buffer legalization, which build handles with their own casts.
void diagnoseBitCastsToDescriptorHandle(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
