// slang-ir-eliminate-multi-level-break.h
#pragma once

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;
class TargetProgram;

void eliminateMultiLevelBreak(IRModule* module, TargetProgram* targetProgram);
void eliminateMultiLevelBreakForFunc(
    TargetProgram* targetProgram,
    IRModule* module,
    IRGlobalValueWithCode* func);

} // namespace Slang
