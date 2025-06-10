#ifndef SLANG_IR_UNDO_PARAM_COPY_H
#define SLANG_IR_UNDO_PARAM_COPY_H

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
// Replace temporary variables created for parameter passing with direct pointer access
// This is particularly important for CUDA/OptiX targets where functions like 'IgnoreHit'
// prevent the copy-back step from executing for inout parameters
void undoParameterCopy(IRModule* module);
} // namespace Slang

#endif