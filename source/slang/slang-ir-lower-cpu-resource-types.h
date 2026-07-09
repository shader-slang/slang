// slang-ir-lower-cpu-resource-types.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
struct CodeGenContext;

/// Lowers resource types (buffers, textures, acceleration structures) to
/// concrete types on CPU targets. Unlike GPU targets, CPU targets don't have
/// any built-in "Textures" or "StructuredBuffers", so they must be turned into
/// something concrete, such as pointers.
///
void lowerCPUResourceTypes(IRModule* module, CodeGenContext* codeGenContext);

} // namespace Slang
