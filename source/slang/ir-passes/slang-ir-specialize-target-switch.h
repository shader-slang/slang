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
// `missingReturn` with a `NoTargetCaseDecoration`, located at the switch. We do not report it
// here, because linking also pulls in functions that are referenced only from another target's
// case, such as the metal-only helper behind the `metal` case of `sincos`, and DCE removes those
// later. Instead, E41037 is reported late, in `diagnoseRemainingNoTargetCases`, after the last
// DCE of `linkAndOptimizeIR`, so that only code that will actually be emitted is diagnosed. A
// case-less switch in a void function is left bare: an empty body is a well-defined no-op there.
// Targets that reject missing returns (SPIR-V, GLSL, WGSL) already error on the bare
// `missingReturn` with E41009 before that point, unless
// `-disable-non-essential-validations` skipped the check, in which case the late report covers
// them too.
//
void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

// Report E41037 for every `NoTargetCaseDecoration` left in the top-level functions of `module`.
// Bodies inside a surviving `IRGeneric` are not examined, because no target emits code for them.
//
void diagnoseRemainingNoTargetCases(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

} // namespace Slang

#endif
