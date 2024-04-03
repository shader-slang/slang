// slang-ir-variable-scope-correction.h
#ifndef SLANG_IR_VARIABLE_SCOPE_CORRECTION_H
#define SLANG_IR_VARIABLE_SCOPE_CORRECTION_H

namespace Slang
{

struct IRModule;

/// Correct the scope of variables in the IR module.
void applyVariableScopeCorrection(IRModule* module);

}

#endif // SLANG_IR_VARIABLE_SCOPE_CORRECTION_H
