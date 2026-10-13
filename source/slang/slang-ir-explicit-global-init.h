// slang-ir-explicit-global-init.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
class TargetProgram;

/// Move global initializers required by the target or by resource-global replacement into each
/// defined module-scope shader entry point and CUDA kernel.
///
/// Selected globals retain their storage but no longer contain initializer bodies. `ActualGlobal`
/// variables are excluded because their storage lifetime spans entry-point invocations.
/// Native host targets retain module-scope initialization except for selected resource globals.
/// HLSL retains declaration initializers except for arrays, cooperative vectors, and selected
/// resource globals; other kernel targets move every non-`ActualGlobal` initializer.
///
/// Selected resource globals represent either programmer-declared mutable `static` resource
/// variables at file or namespace scope, or mutable uniform-parameter shadows synthesized for
/// `-Gec`. For those globals, the operation requires a proof that evaluating the initializer at
/// each entry point cannot change observable behavior. The proof requires the initializer and
/// every function it can call to return normally for all possible inputs. It rejects
/// externally observable side effects and reads of preexisting mutable storage, resource contents,
/// or non-resource data from an explicit parameter group. If any selected resource initializer
/// fails this proof, the operation diagnoses all failures and leaves the module unchanged. `sink`
/// must not be null. This operation requires completion of every resource-type or empty-type
/// legalization selected for the target and entry-point call-site splitting. It must run before
/// `legalizeResourceGlobalVars` and array-return legalization.
void moveGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);
} // namespace Slang
