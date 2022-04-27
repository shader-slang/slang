// slang-ir-liveness.h
#ifndef SLANG_IR_LIVENESS_H
#define SLANG_IR_LIVENESS_H

namespace Slang
{

struct IRModule;

void addLivenessTracking(IRModule* module);

}

#endif // SLANG_IR_LIVENESS_H