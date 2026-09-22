// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

namespace Slang
{

class DiagnosticSink;
struct IRModule;
class TargetProgram;

/// Replaces file-scope `static` resource globals whose values belong to one shader invocation with
/// explicit entry-point-local state.
///
/// This operation first moves resource-dependent initialization into each entry point, then threads
/// each localized value through helper parameters. It diagnoses externally observable storage,
/// invocations without a rewritable direct call site, escaping addresses, and calls whose explicit
/// arguments alias implicitly threaded state, because localization could not preserve those
/// contracts or storage identities.
///
/// This operation must run after linking, because linkage, direct calls, and function references
/// expose the boundaries it validates. It must run before resource-type legalization, while each
/// file-scope `static` and its initializer still form one identifiable global value.
void legalizeResourceGlobalVars(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);

} // namespace Slang

#endif
