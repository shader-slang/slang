#include "slang-ir-cuda-noinline.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-target.h"

namespace Slang
{

// Count the instructions in a function's body across all of its blocks, including block parameters
// and terminators. This is a cheap, stable proxy for the size of the CUDA source the function will
// emit to; it does not need to match the emitted line count exactly, only to order functions by
// size consistently so a single threshold can separate the large ones.
static IRIntegerValue getFunctionBodyInstCount(IRFunc* func)
{
    IRIntegerValue count = 0;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            SLANG_UNUSED(inst);
            count++;
        }
    }
    return count;
}

// Decide whether `func` is an ordinary `__device__` function that the CUDA emitter would emit a
// `__noinline__` specifier for once it carries `IRNoInlineDecoration`.
//
// The CUDA emitter (`CUDASourceEmitter::emitFunctionPreambleImpl`) classifies every function it
// emits into exactly one specifier: an entry point becomes `extern "C" __global__`, an
// `[CudaKernel]` becomes `__global__`, a `[CudaHost]` becomes `__host__`, and everything else
// becomes `__device__` — and only that final branch emits `__noinline__`. So marking a kernel or
// host function would attach a decoration the emitter never reads; a kernel is also a call-graph
// root with no caller to be inlined into. We likewise leave a function that already requests
// inlining alone rather than emit a contradictory hint. Intrinsics are excluded using the same
// target-aware test the emitter uses to decide whether to emit a function at all
// (`CLikeSourceEmitter::isTargetIntrinsic` / `findTargetIntrinsicDefinition`), so a function that
// is an intrinsic on a *different* target but a real device function here is still treated as
// ordinary.
static bool isOrdinaryCUDADeviceFunction(IRFunc* func, CapabilitySet const& targetCaps)
{
    if (!func->isDefinition())
        return false;
    if (func->findDecoration<IREntryPointDecoration>() ||
        func->findDecoration<IRCudaKernelDecoration>() ||
        func->findDecoration<IRCudaHostDecoration>())
        return false;
    if (func->findDecoration<IRForceInlineDecoration>() ||
        func->findDecoration<IRUnsafeForceInlineEarlyDecoration>())
        return false;
    UnownedStringSlice intrinsicDef;
    IRInst* intrinsicInst;
    if (Slang::findTargetIntrinsicDefinition(func, targetCaps, intrinsicDef, intrinsicInst))
        return false;
    return true;
}

void markLargeCUDADeviceFunctionsNoInline(
    IRModule* module,
    TargetProgram* targetProgram,
    int threshold)
{
    if (threshold <= 0)
        return;

    auto targetCaps = targetProgram->getTargetReq()->getTargetCaps();

    IRBuilder builder(module);
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;
        if (func->findDecoration<IRNoInlineDecoration>())
            continue;
        if (!isOrdinaryCUDADeviceFunction(func, targetCaps))
            continue;
        if (getFunctionBodyInstCount(func) > threshold)
            builder.addSimpleDecoration<IRNoInlineDecoration>(func);
    }
}

} // namespace Slang
