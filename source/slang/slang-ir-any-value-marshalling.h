// slang-ir-any-value-marshalling.h
#pragma once

#include "../core/slang-common.h"

namespace Slang
{
struct IRType;
struct IRModule;
struct CompilerOptionSet;
class TargetProgram;

/// Generates functions that pack and unpack `AnyValue`s, and replaces
/// all `IRPackAnyValue` and `IRUnpackAnyValue` instructions with calls
/// to these packing/unpacking functions.
/// This is a sub-pass of lower-generics.
void generateAnyValueMarshallingFunctions(IRModule* module, TargetProgram* targetProgram);


/// Get the AnyValue size required to hold a value of `type`.
SlangInt getAnyValueSize(CompilerOptionSet& optionSet, IRType* type);
} // namespace Slang
