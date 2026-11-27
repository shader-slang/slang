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
// A `__target_switch` with no case for the target in a value-returning function becomes a
// `missingReturn` with a `NoTargetCaseDecoration`, located at the switch. The switch cannot be
// diagnosed here: linking also pulls in functions that are referenced only from another target's
// case, such as the metal-only helper behind the `metal` case of `sincos`, and DCE removes those
// later. E41037 is instead reported late, in `linkAndOptimizeIR`, for every mark that survives the
// last force-inlining and full simplifyIR passes, so that only code that will actually be emitted
// is diagnosed.
//
// A case-less switch in a void function is left bare, because an empty body is a well-defined
// no-op there: those functions compile exactly as before. On targets that reject missing returns
// (SPIR-V, GLSL, WGSL) every `missingReturn`, bare or marked, still gets the pre-existing E41009
// from the early `checkForMissingReturns`, unless `-disable-non-essential-validations` skipped
// that check, in which case the late report covers those targets too. The `slangvm` target never
// reaches the late report: its capability profile is empty, so a switch with no case for it
// errors E41011 here at link time instead.
//
void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

// Report E41037 for every `NoTargetCaseDecoration` left in the top-level functions of `module`.
// Bodies inside a surviving `IRGeneric` are not examined, because no target emits code for them.
// `target` is used only to name the target in the diagnostic; it does not gate which functions
// are scanned.
//
void diagnoseRemainingNoTargetCases(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

} // namespace Slang

#endif
