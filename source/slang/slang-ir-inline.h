// slang-ir-inline.h
#pragma once

#include "../../slang-com-helper.h"

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRGlobalValueWithCode;
    class DiagnosticSink;

        /// Any call to a function that takes or returns a string parameter is inlined
    Result performStringInlining(IRModule* module, DiagnosticSink* sink);

        /// Inline any call sites to functions marked `[unsafeForceInlineEarly]`
    void performMandatoryEarlyInlining(IRModule* module);

        /// Inline any call sites to functions marked `[ForceInline]`
    void performForceInlining(IRModule* module);

        /// Inline any call sites to functions marked `[ForceInline]` inside `func`.
    bool performForceInlining(IRGlobalValueWithCode* func);
    
        /// Perform force inlining of functions that does not have custom derivatives.
    bool performPreAutoDiffForceInlining(IRGlobalValueWithCode* func);

        /// Perform force inlining of all functions in a module that does not have custom derivatives.
    bool performPreAutoDiffForceInlining(IRModule* module);

        /// Inline calls to functions that returns a resource/sampler via either return value or output parameter.
    void performGLSLResourceReturnFunctionInlining(IRModule* module);

        /// Inline simple intrinsic functions whose definition is a single asm block.
    void performIntrinsicFunctionFunctionInlining(IRModule* module);

        /// Inline a specific call.
    bool inlineCall(IRCall* call);
}
