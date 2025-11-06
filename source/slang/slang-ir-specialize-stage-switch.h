#ifndef SLANG_IR_SPECIALIZE_STAGE_SWITCH_H
#define SLANG_IR_SPECIALIZE_STAGE_SWITCH_H

namespace Slang
{

struct TargetRequest;
struct IRModule;

// Repalce all stage_switch insts with the case that matches current calling entrypoint.
//
void specializeStageSwitch(TargetRequest* targetRequest, IRModule* module);

} // namespace Slang

#endif
