#pragma once

namespace Slang
{

/*
This pass will lower all immutable buffer loads into CUDA `__ldg` intrinsic calls
to make sure these loads are performed through the read-only data cache on the GPU
for better performance.
*/

struct IRModule;
class TargetProgram;

void lowerImmutableBufferLoadForCUDA(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
