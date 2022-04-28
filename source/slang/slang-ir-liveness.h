// slang-ir-liveness.h
#ifndef SLANG_IR_LIVENESS_H
#define SLANG_IR_LIVENESS_H

namespace Slang
{

struct IRModule;

/// Adds LiveStart and LiveEnd instructions to demark the start and end of the liveness of a variable.
void addLivenessTrackingToModule(IRModule* module);

}

#endif // SLANG_IR_LIVENESS_H