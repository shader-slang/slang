#pragma once

namespace Slang
{
struct IRModule;

/// Replaces the CUDA TensorView descriptor and its typed queries with ordinary values before
/// NVVM entry/helper and physical storage legalization.
void lowerNVVMTensorViews(IRModule* module);
} // namespace Slang
