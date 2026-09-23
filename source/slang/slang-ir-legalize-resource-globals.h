// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

namespace Slang
{

class DiagnosticSink;
struct IRModule;

/// Replace each global selected by `isFileScopeStaticResourceGlobalToReplace` with local storage in
/// every entry point that may access it.
///
/// Require selected initializer bodies to have been moved by
/// `moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization`. Add a generated parameter
/// to each affected non-entry-point function, and add the corresponding argument to every in-module
/// direct call. Diagnose uses that require one persistent address or that invoke an affected
/// function without a direct `IRCall` that can receive the generated argument.
///
/// Run this operation after linking but before resource-type legalization and
/// `specializeResourceUsage`. At that point all relevant in-module functions and direct calls are
/// available. The later legalization and resource-specialization passes can then consume the
/// generated locals, parameters, and call arguments.
void legalizeResourceGlobalVars(IRModule* module, DiagnosticSink* sink);

} // namespace Slang

#endif
