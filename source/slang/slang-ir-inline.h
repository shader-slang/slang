// slang-ir-inline.h
#pragma once

#include "core/slang-basic.h"
#include "slang-com-helper.h"

namespace Slang
{
struct IRModule;
struct IRCall;
struct IRGlobalValueWithCode;
class DiagnosticSink;
class TargetProgram;
struct IRInst;

/// Inline calls whose parameter or result types require it on the selected target, including
/// strings, references, and HLSL output aggregates containing matrices with non-default layouts.
Result performTypeInlining(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

/// Inline any call sites to functions marked `[unsafeForceInlineEarly]`
bool performMandatoryEarlyInlining(IRModule* module, HashSet<IRInst*>* modifiedFuncs = nullptr);

/// Inline any call sites to functions marked `[ForceInline]`
void performForceInlining(IRModule* module);

/// Inline any call sites to functions marked `[ForceInline]` inside `func`.
bool performForceInlining(IRGlobalValueWithCode* func);

/// Perform force inlining of functions that does not have custom derivatives.
bool performPreAutoDiffForceInlining(IRGlobalValueWithCode* func);

/// Perform force inlining of all functions in a module that does not have custom derivatives.
bool performPreAutoDiffForceInlining(IRModule* module);

/// Inline calls to functions that returns a resource/sampler via either return value or output
/// parameter.
void performGLSLResourceReturnFunctionInlining(IRModule* module, TargetProgram* targetProgram);

/// Inline simple intrinsic functions whose definition is a single asm block.
void performIntrinsicFunctionInlining(IRModule* module);

/// Inline a specific call.
bool inlineCall(IRCall* call);
} // namespace Slang
