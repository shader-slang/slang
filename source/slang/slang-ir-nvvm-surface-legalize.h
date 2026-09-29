#pragma once

namespace Slang
{
struct IRModule;

/// Makes physical surface accesses and their format conversions explicit before component masks
/// are discarded. This pass is scheduled only for the direct NVVM route.
void legalizeNVVMSurfaceOperations(IRModule* module);
} // namespace Slang
