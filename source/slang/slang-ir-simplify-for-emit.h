// slang-ir-simplify-for-emit.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetRequest;

void simplifyForEmit(IRModule* inModule, TargetRequest* req);

// Drop `kIROp_CastToVoid` insts (no backend handles them; the op is produced
// by `(void)expr` source casts). Safe to run on the linked IR module before
// any target-specific emit step.
void eliminateCastToVoid(IRModule* module);
} // namespace Slang
