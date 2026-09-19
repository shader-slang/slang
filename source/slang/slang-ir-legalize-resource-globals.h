// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

namespace Slang
{

class DiagnosticSink;
struct IRModule;
class TargetProgram;

/// Legalize per-invocation source-static resource state held in global IR storage into explicit
/// entry-point state.
///
/// This operation moves resource-dependent initialization into each entry point, replaces the
/// affected resource globals with entry-point locals, and threads their values through helper
/// parameters. It diagnoses externally preserved storage and invocation boundaries that cannot
/// carry per-invocation state without changing their contract.
///
/// Invoke it after linking, while source-static identities are intact, and before resource-type
/// legalization and resource-usage specialization.
void legalizeResourceGlobalVars(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);

} // namespace Slang

#endif
