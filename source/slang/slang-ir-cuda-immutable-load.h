#pragma once

namespace Slang
{

/*
This pass lowers immutable buffer loads into CUDA `__ldg` intrinsic calls so they
can use the GPU read-only data cache.

It also lowers aligned device-pointer loads and stores by creating wrapper struct
types with alignment decorations. Loads bitcast the source pointer to the wrapper
type and extract the wrapper field after loading; stores build the same wrapper
value and write it through a bitcast destination pointer. Keeping these transforms
in one pass lets the immutable-load path compose with the aligned wrapper shape.
*/

struct IRModule;
class TargetProgram;

void lowerImmutableOrAlignedBufferLoadForCUDA(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
