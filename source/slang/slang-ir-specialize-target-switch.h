#ifndef SLANG_IR_SPECIALIZE_TARGET_SWITCH_H
#define SLANG_IR_SPECIALIZE_TARGET_SWITCH_H

namespace Slang
{
    struct IRModule;
    class TargetRequest;

    // Repalce all target_switch insts with the case that matches current target.
    //
    void specializeTargetSwitch(TargetRequest* target, IRModule* module);

}

#endif
