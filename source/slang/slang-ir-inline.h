// slang-ir-inline.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;

        /// Inline any call sites to functions marked `[unsafeForceInlineEarly]`
    void performMandatoryEarlyInlining(IRModule* module);

        /// Inline calls to functions that returns a resource/sampler via either return value or output parameter.
    void performGLSLResourceReturnFunctionInlining(IRModule* module);

        /// Inline a specific call.
    bool inlineCall(IRCall* call);
}
