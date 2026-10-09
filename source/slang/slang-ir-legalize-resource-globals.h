// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

namespace Slang
{

class DiagnosticSink;
struct CapabilitySet;
struct IRModule;

/// Return whether `module` contains an `IRGlobalVar` for which
/// `isResourceGlobalCandidateForPerInvocationReplacement` returns true.
///
/// This query does not perform the legalizer's whole-module validation. A true result therefore
/// means that the caller must prepare for the pass, not that the pass will necessarily succeed.
bool doesModuleContainResourceGlobalCandidate(IRModule* module);

/// Replace mutable resource state that has one value per entry-point invocation with
/// function-local storage.
///
/// A selected global is an `IRGlobalVar` marked with
/// `IRFileOrNamespaceScopeMutableVarDecoration` whose rate and stored type are accepted by
/// `isResourceGlobalCandidateForPerInvocationReplacement`. Lowering applies the marker to mutable
/// file- or namespace-scope `static` variables and mutable shadows of HLSL uniform parameters
/// synthesized for `-Gec`.
///
/// When `shouldDiagnoseUninitializedValues` is true,
/// `checkForUsingUninitializedValues` must have run before linking. Linking,
/// unreachable-control-flow elimination, and any resource- or empty-type legalization selected for
/// the target must also have completed.
/// `moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization` must have removed every
/// selected global's initializer body and inserted the corresponding stores at defined entry
/// points. `fixEntryPointCallsites` must have redirected every ordinary call to a shader entry
/// point or CUDA kernel. This operation rejects an affected entry point when an invocation
/// decoration describes a call with no extensible `IRCall`, or when passing, storing, or returning
/// its function value may lead to an indirect call that this operation cannot find and rewrite.
/// `specializeResourceUsage` must run afterward.
///
/// The operation validates all selected globals and affected functions before changing the module.
/// Unsupported cases include storage that must remain at module scope; a reference outside a
/// function body; an external reference to metadata that would be removed with the global; an
/// invocation whose arguments cannot be extended; an emitted implementation that replaces the
/// analyzed IR body; an address use that may escape or observe storage identity; a resource array
/// whose initialization would require subobject tracking; and conflicting writable call arguments.
/// If any such check fails, the operation reports diagnostics and leaves the module unchanged.
///
/// The operation gives each entry point whose execution may read or write a selected value a local
/// for that value. It gives each non-entry-point function whose execution may read or write the
/// value a generated parameter and replacement local, and extends every direct call to that
/// function with the corresponding argument. A function with no direct or transitive runtime
/// access receives a local but no parameter when its body directly references the global in
/// non-runtime IR. In this contract, an entry point is either a shader entry point or a CUDA
/// kernel. When `shouldDiagnoseUninitializedValues` is true, the operation checks any selected
/// global that still has a possible write or carries the marker recorded when the pre-link check
/// found a possible write. It diagnoses an entry-point read that can execute before its replacement
/// local has been fully assigned. It removes the selected globals even when it emits such a
/// diagnostic. `specializeResourceUsage` can subsequently apply its normal input- and
/// output-specialization rules to the generated parameters.
void legalizeResourceGlobalVars(
    IRModule* module,
    CapabilitySet const& targetCaps,
    bool shouldDiagnoseUninitializedValues,
    DiagnosticSink* sink);

} // namespace Slang

#endif
