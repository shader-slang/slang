// slang-ir-explicit-global-init.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
class TargetProgram;

/// Move each eligible global initializer selected by the target policy to the start of every
/// defined entry point.
///
/// Selected globals retain their storage but no longer contain initializer bodies. A variable
/// whose lifetime extends beyond one entry-point invocation is not eligible for this operation.
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram);

/// Move eligible file-scope `static` resource initializers to every defined entry point.
///
/// Require each moved initializer to have no side effects and not to read mutable state.
/// This restriction makes the initializer independent of its position among the initializers that
/// remain at global scope. Diagnose an initializer that does not meet this requirement. Call this
/// operation immediately before replacing the selected global storage declarations.
void moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
    IRModule* module,
    DiagnosticSink* sink);
} // namespace Slang
