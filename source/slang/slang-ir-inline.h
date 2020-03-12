// slang-ir-inline.h
#pragma once

namespace Slang
{
    struct IRModule;

        /// Inline any call sites to functions marked `[unsafeForceInlineEarly]`
    void performMandatoryEarlyInlining(IRModule* module);
}
