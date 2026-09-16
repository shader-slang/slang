// slang-ir-legalize-resource-globals.h
#ifndef SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H
#define SLANG_IR_LEGALIZE_RESOURCE_GLOBALS_H

#include "core/slang-list.h"

namespace Slang
{

class DiagnosticSink;
struct IRGlobalVar;
struct IRModule;

/// Replace per-invocation file-scope variables whose types contain resource handles with
/// entry-point locals and threaded parameters.
///
/// Resource-dependent initializers must already have been moved into entry points.
/// `resourceDependentState` must be the complete list produced by that move, including resource
/// globals, initializer targets, and ordinary globals the moved initializer call graph may mutate.
/// The pass diagnoses preserved call/storage boundaries before rewriting any state. Resource types
/// must not yet have been split into leaf variables. The module must contain linked source globals,
/// identified by their linkage decorations, and calls between defined functions must be direct.
/// Existing resource specialization must run afterward; target-specific restrictions can still
/// reject resource value-flow shapes that it cannot resolve.
void legalizeResourceGlobalVars(
    IRModule* module,
    List<IRGlobalVar*> const& resourceDependentState,
    DiagnosticSink* sink);

} // namespace Slang

#endif
