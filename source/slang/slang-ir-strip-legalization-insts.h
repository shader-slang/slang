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

/// Remove every [KeepAlive] decoration from every top-level generic, whatever added it, and return
/// whether any were removed. Dead-code elimination can then remove each generic nothing uses.
///
/// The caller must have finished every pass that diagnoses unspecialized generic bodies, such as
/// `legalizeResourceGlobalVars` in `linkAndOptimizeIR`, because a removed generic is no longer
/// checked. Most pins come from `export` (for example a user `getDescriptorFromHandle` override),
/// which keeps a generic alive so that linking can bind it; no emitter can emit a generic.
bool unpinGenerics(IRModule* module);

} // namespace Slang
