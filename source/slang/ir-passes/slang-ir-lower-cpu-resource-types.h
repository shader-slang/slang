// slang-ir-lower-cpu-resource-types.h
#pragma once

#include "slang/ir/slang-ir.h"

namespace Slang
{
struct IRModule;
struct CodeGenContext;

/// Lowers resource types (buffers, textures, acceleration structures) to
/// concrete types on CPU targets. Unlike GPU targets, CPU targets don't have
/// any built-in "Textures" or "StructuredBuffers", so they must be turned into
/// something concrete, such as pointers.
/// Sized buffers lower to a data pointer and count; unsized buffers lower to a
/// struct with only the data pointer, matching the C++ prelude representation.
void lowerCPUResourceTypes(IRModule* module, CodeGenContext* codeGenContext);

} // namespace Slang
