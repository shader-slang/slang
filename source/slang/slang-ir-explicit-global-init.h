// slang-ir-explicit-global-init.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
class TargetProgram;

/// Move each global initializer selected by `targetProgram`'s module-scope initialization rules to
/// the start of every defined module-scope entry point.
///
/// Selected globals retain their storage but no longer contain initializer bodies. `ActualGlobal`
/// variables are excluded because their storage lifetime spans entry-point invocations.
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram);

/// Move each initializer for a resource-valued `static` variable declared at file or namespace
/// scope to every defined module-scope shader entry point and CUDA kernel.
///
/// The operation moves an initializer only if the analysis proves that evaluating it at the start
/// of each entry point cannot change observable behavior. The proof rejects externally observable
/// side effects and reads of preexisting mutable storage, resource contents, or non-resource data
/// from a source-declared parameter group. If any selected initializer fails this proof, the
/// operation diagnoses all failures and leaves the module unchanged. Call this function after any
/// resource- or empty-type legalization selected for the target and immediately before
/// `legalizeResourceGlobalVars`.
void moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);
} // namespace Slang
