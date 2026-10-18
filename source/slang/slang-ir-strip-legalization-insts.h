// slang-ir-strip-legalization-insts.h
#pragma once

namespace Slang
{

struct IRModule;

/// Removes global instructions from the module that are only required for legalization.
/// These instructions are safe to or must be removed for instruction emitting.
///
/// Currently does the following:
/// - Removes specialization dictionaries.
/// - Removes the contents of all witness table instructions.
/// - Removes global param entry point decorations, as they will cause false code-gen circularity
///   alerts during pre-emit actions.
///
void stripLegalizationOnlyInstructions(IRModule* module);

/// Remove [KeepAlive] decorations from witness tables.
void unpinWitnessTables(IRModule* module);

/// Remove [KeepAlive] decorations from top-level generics, and return whether any were removed.
///
/// An `export`ed generic (for example a user `getDescriptorFromHandle` override) is pinned so that
/// it survives linking and the checks in `linkAndOptimizeIR`, some of which diagnose its
/// unspecialized body (for example `legalizeResourceGlobalVars`). We therefore call this only after
/// `linkAndOptimizeIR`. By then no emitter can emit a generic, so the pin only keeps an
/// unspecialized body alive for target legalization that cannot handle it. Unpinning lets
/// dead-code elimination remove the generic unless something still uses it.
bool unpinGenerics(IRModule* module);

} // namespace Slang
