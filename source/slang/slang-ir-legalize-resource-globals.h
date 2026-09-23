// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

namespace Slang
{

class DiagnosticSink;
struct CapabilitySet;
struct IRModule;

/// Replace each global selected by `isFileOrNamespaceScopeStaticResourceGlobalToReplace` with
/// per-function locals. Add generated parameters and direct-call arguments so that
/// non-entry-point functions can receive each value they may read or write.
///
/// Require selected initializer bodies to have been moved by
/// `moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization`. Give each
/// non-entry-point function with a direct or transitive runtime read or write a generated parameter
/// and local copy. A function with a direct non-runtime reference and no runtime access receives
/// only a local. Add the corresponding argument to every in-module direct call whose callee gains
/// a parameter. Diagnose entry-point paths that read the value before it has been initialized.
/// Diagnose selected globals whose linkage or retention requirements prevent replacement with
/// function-local storage. Diagnose address uses that may retain the address or observe its storage
/// identity; separate function-local variables cannot preserve one shared identity. Diagnose any
/// resource-using non-entry-point function with an invocation that is not an in-module direct
/// `IRCall`. Diagnose a resource-using entry point if the module also uses it as a callable
/// function. Reject access from a function when generic assembly or a target-intrinsic definition
/// selected by `targetCaps` supplies the emitted implementation instead of the function's IR
/// blocks.
///
/// Run this operation after linking, unreachable-control-flow elimination, and any resource- or
/// empty-type legalization selected for the target, but before `specializeResourceUsage`. The
/// operation treats every remaining direct call between functions in the module as a possible
/// invocation and rewrites it when the callee gains a parameter. The later resource-specialization
/// pass can then apply its existing input- and output-specialization rules to the generated
/// resource parameters.
void legalizeResourceGlobalVars(
    IRModule* module,
    CapabilitySet const& targetCaps,
    DiagnosticSink* sink);

} // namespace Slang

#endif
