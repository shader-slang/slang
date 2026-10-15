#ifndef SLANG_IR_SPECIALIZE_TARGET_SWITCH_H
#define SLANG_IR_SPECIALIZE_TARGET_SWITCH_H

namespace Slang
{
struct IRModule;
struct IRNoTargetCaseDecoration;
class TargetRequest;
class DiagnosticSink;
enum class CodeGenTarget;

// Replace all target_switch insts with the case that matches the current target.
//
// A `__target_switch` with no case for the target becomes a `missingReturn` with a
// `NoTargetCaseDecoration`, located at the switch. We do not report it here, because linking
// also pulls in functions that are referenced only from another target's case, such as
// `__sincos_metal` from the `metal` case of `sincos`, and DCE removes those later. Instead,
// E41037 is reported from one of two places:
//
// - On targets that reject missing returns (SPIR-V, GLSL, WGSL), `checkForMissingReturns`
//   reports it in place of E41009. Every `missingReturn` there was already an error before
//   E41037 existed, so this only changes the message and keeps the error-count check that
//   follows it.
//
// - Everywhere else, and on those targets when `-disable-non-essential-validations` skips that
//   check, `diagnoseRemainingNoTargetCases` reports it late in `linkAndOptimizeIR`. It runs after
//   target legalization such as `legalizeImageSubscript`, which rewrites some core-module calls
//   whose switch has no case for the target, so that only code that will be emitted is reported.
//
void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

// Report E41037 for the `missingReturn` that carries `noTargetCase`, naming the function recorded
// in the decoration, followed by "see call to" notes along the first call path from the function
// that currently contains it.
//
void diagnoseNoTargetCase(
    IRNoTargetCaseDecoration* noTargetCase,
    CodeGenTarget target,
    DiagnosticSink* sink);

// Report E41037 for every `NoTargetCaseDecoration` left in the top-level functions of `module`.
// Bodies inside a surviving `IRGeneric` are not examined, because no target emits code for them.
//
void diagnoseRemainingNoTargetCases(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

} // namespace Slang

#endif
