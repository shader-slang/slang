#ifndef SLANG_IR_SPECIALIZE_TARGET_SWITCH_H
#define SLANG_IR_SPECIALIZE_TARGET_SWITCH_H

namespace Slang
{
struct IRModule;
struct IRMissingReturn;
struct IRNoTargetCaseDecoration;
class TargetRequest;
class DiagnosticSink;
enum class CodeGenTarget;

// Repalce all target_switch insts with the case that matches current target.
//
// A switch with no case for the target becomes a `missingReturn` carrying a
// `NoTargetCaseDecoration`; see that decoration in slang-ir-insts.lua for how it is reported.
//
void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

// Returns the decoration if `missingReturn` replaced a `__target_switch` with no case for the
// target, or null if it is an ordinary missing return.
//
IRNoTargetCaseDecoration* findNoTargetCaseDecoration(IRMissingReturn* missingReturn);

// Report E41037 for `missingReturn`, naming the function recorded in `noTargetCase`, at the
// location of `missingReturn`, followed by "see call to" notes along the first call path from
// the function that currently contains it.
//
void diagnoseNoTargetCase(
    IRMissingReturn* missingReturn,
    IRNoTargetCaseDecoration* noTargetCase,
    CodeGenTarget target,
    DiagnosticSink* sink);

// Report E41037 for every `missingReturn` with a `NoTargetCaseDecoration` left in the top-level
// functions of `module`. Bodies inside a surviving `IRGeneric` are not examined, because no
// target emits code for them. The caller runs this after the dead-code elimination that follows
// resource specialization, so that functions only referenced through another target's case
// (such as `__sincos_metal` on CUDA) have been removed.
//
void diagnoseRemainingNoTargetCases(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

} // namespace Slang

#endif
