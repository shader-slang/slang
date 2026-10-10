#ifndef SLANG_IR_SPECIALIZE_TARGET_SWITCH_H
#define SLANG_IR_SPECIALIZE_TARGET_SWITCH_H

namespace Slang
{
struct IRModule;
struct IRMissingReturn;
class TargetRequest;
class DiagnosticSink;
enum class CodeGenTarget;

// Repalce all target_switch insts with the case that matches current target.
//
void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

// Returns true if `missingReturn` replaced a `__target_switch` that has no case for the target.
//
bool isNoTargetCaseMissingReturn(IRMissingReturn* missingReturn);

// Report that the code ending in `missingReturn`, which must satisfy
// `isNoTargetCaseMissingReturn`, has no implementation for `target`.
//
void diagnoseNoTargetCase(
    IRMissingReturn* missingReturn,
    CodeGenTarget target,
    DiagnosticSink* sink);

// Report an error for every `__target_switch` without a case for `target` whose code is still
// present in `module`. Run after the final dead-code elimination, so that only code that will
// actually be generated is diagnosed.
//
void diagnoseReachableNoTargetCase(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

} // namespace Slang

#endif
