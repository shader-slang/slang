// slang-ir-array-reg-to-mem.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRInst;
    struct IRFunc;

        /// Eliminate SSA registers and IRParams of array type and turn them into pointers to memory objects.
    bool eliminateArrayTypeSSARegisters(IRModule* module);

    bool eliminateArrayTypeParameters(IRFunc* func);

}
