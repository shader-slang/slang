// slang-ir-any-value-marshalling.h
#pragma once

#include "core/slang-common.h"

namespace Slang
{
struct IRType;
struct IRModule;
class DiagnosticSink;
class TargetProgram;
class TargetRequest;

/// Generates functions that pack and unpack `AnyValue`s, and replaces
/// all `IRPackAnyValue` and `IRUnpackAnyValue` instructions with calls
/// to these packing/unpacking functions.
/// A packed or unpacked type whose representation on the target does not match its
/// AnyValue layout (a combined texture-sampler on Metal) is diagnosed into `sink` instead.
/// This is a sub-pass of lower-generics.
void generateAnyValueMarshallingFunctions(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);


/// Get the AnyValue size required to hold a value of `type`.
SlangInt getAnyValueSize(IRType* type, TargetRequest* targetReq);
} // namespace Slang
