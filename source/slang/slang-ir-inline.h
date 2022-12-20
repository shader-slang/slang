// slang-ir-inline.h
#pragma once

#include "../../slang-com-helper.h"

namespace Slang
{
    struct IRModule;
    struct IRCall;

    class DiagnosticSink;

        /// Any call to a function that takes or returns a string parameter is inlined
    Result performStringInlining(IRModule* module, DiagnosticSink* sink);

        /// Inline any call sites to functions marked `[unsafeForceInlineEarly]`
    void performMandatoryEarlyInlining(IRModule* module);

        /// Inline any call sites to functions marked `[ForceInline]`
    void performForceInlining(IRModule* module);

        /// Inline calls to functions that returns a resource/sampler via either return value or output parameter.
    void performGLSLResourceReturnFunctionInlining(IRModule* module);

        /// Inline a specific call.
    bool inlineCall(IRCall* call);
}
