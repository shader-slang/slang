#pragma once

namespace Slang
{

struct IRModule;
class TargetProgram;

// Attach `IRNoInlineDecoration` to every ordinary device function in `module` whose IR body exceeds
// `threshold` instructions, so the CUDA emitter spells it `__noinline__`. A non-positive
// `threshold` leaves the module untouched. Must run after the last CUDA-applicable force-inlining
// pass and is gated to the CUDA target family by its caller, since other backends also consume
// `IRNoInlineDecoration`.
void markLargeCUDADeviceFunctionsNoInline(
    IRModule* module,
    TargetProgram* targetProgram,
    int threshold);

} // namespace Slang
