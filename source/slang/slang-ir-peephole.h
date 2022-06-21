// slang-ir-peephole.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;

        /// Apply peephole optimizations.
    bool peepholeOptimize(IRModule* module);
}
